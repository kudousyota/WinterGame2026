#include "EnemyStateAttack.h"
#include "EnemyStateRetreat.h"
#include "../../../../DataLoader/DataManager.h"
#include "../../../../Effect/EffectManager.h"
#include <cstdio>
#include <cmath>
#include <algorithm>

namespace
{
	//予兆エフェクトの拡大率
	constexpr float kTelegraphScale = 20.0f;
}

EnemyStateAttack::EnemyStateAttack(Enemy& enemy) :
	EnemyStateBase(enemy),
	m_info(DataManager::GetInstance().GetAttackTable().GetInfo(enemy.GetAttackType()))
{
}

void EnemyStateAttack::Enter()
{
	//攻撃中はその場で止まる
	m_enemy.StopMove();
	//攻撃を始めた時の方向に攻撃する
	m_enemy.FaceTarget();
	//予備動作(振りかぶり)がある攻撃は、最初はゆっくり再生して溜めているように見せる
	const float animScale = (m_info.windupFrame > 0) ? m_info.windupAnimScale : 1.0f;
	m_enemy.GetAnim().ChangeAnim(m_info.animName, false, animScale);
	m_attackId = m_enemy.IssueAttackId();
	m_nextHitFrame = static_cast<float>(m_info.hitStartFrame + m_info.hitInterval);

	//予兆が始まった瞬間に狙いを決める
	m_aimDir = CalcAimDir();
	//予兆の始まり
	m_enemy.SetWarning(true);

	//予兆エフェクトを口の位置に出す
	const Vector3& forward = m_enemy.GetForward();
	const Vector3 mouthPos = m_enemy.GetPos() + forward * m_enemy.GetHitForwardOffset() + Vector3(0.0f, m_enemy.GetHitHeight(), 0.0f);
	//エフェクトを敵の向きに合わせる(プレイヤーのエフェクトと同じ求め方)
	Vector3 rot;
	rot.y = atan2f(-forward.x, -forward.z);
	m_telegraphHandle = EffectManager::Instns().PlayEffect(EffectType::AttackTelegraph, mouthPos, rot, kTelegraphScale);
}

void EnemyStateAttack::Update()
{
	m_enemy.StopMove();

	//攻撃が終わったらプレイヤーから離れる
	if (m_frame >= m_info.totalFrame)
	{
		ChangeState<EnemyStateRetreat>();
		return;
	}

	//予備動作が終わったら普通の速さに戻して一気に振る
	if (m_info.windupFrame > 0 && m_frame >= m_info.windupFrame)
	{
		m_enemy.GetAnim().SetAnimTimeScale(1.0f);
	}

	//判定が出るまでが予兆
	m_enemy.SetWarning(m_frame < m_info.hitStartFrame);
	//判定が出たら予兆エフェクトは消す
	if (!m_enemy.IsWarning())
	{
		StopTelegraph();
	}

	//多段ヒットの攻撃は、間隔ごとに新しい番号にして同じ相手にもう一度当たるようにする
	if (m_info.hitInterval > 0 && m_frame >= m_nextHitFrame)
	{
		m_attackId = m_enemy.IssueAttackId();
		m_nextHitFrame += static_cast<float>(m_info.hitInterval);
	}

	if (IsHitActive())
	{
		//攻撃を申請するだけ
		AttackRequest request{ AttackData(CharacterType::Enemy, m_info.type, m_info.damage, m_info.radius) };
		request.attackId = m_attackId;
		request.forwardOffset = m_enemy.GetHitForwardOffset();
		request.height = m_enemy.GetHitHeight();
		//lengthが0なら今まで通りの球、0より大きければ狙った方向へ伸びるカプセルになる
		request.dir = m_aimDir;
		request.length = CalcHitLength();
		m_enemy.RequestAttack(request);
	}

	m_frame += m_enemy.GetTimeScale();
}

void EnemyStateAttack::Exit()
{
	//攻撃の途中でのけぞったときも予兆が出っぱなしにならないようにする
	m_enemy.SetWarning(false);
	StopTelegraph();
	//攻撃が終わったので、ほかの敵が攻撃できるようにする
	m_enemy.ReleaseAttackToken();
}

void EnemyStateAttack::StopTelegraph()
{
	//再生していないときや再生が終わっているときは、EffectManagerの方で何もしない
	EffectManager::Instns().StopEffect(m_telegraphHandle);
}

#ifdef _DEBUG
std::string EnemyStateAttack::GetDebugInfo() const
{
	char buf[160];
	//判定が出ていない間は長さ0として表示する
	const float length = IsHitActive() ? CalcHitLength() : 0.0f;
	sprintf_s(buf, "%s  Frame:%.1f/%d  Hit:%s  Id:%d  Len:%.0f/%.0f",
		m_info.animName.c_str(), m_frame, m_info.totalFrame, IsHitActive() ? "ON" : "OFF", m_attackId,
		length, m_info.length);
	return buf;
}
#endif

float EnemyStateAttack::CalcHitLength() const
{
	//伸びない攻撃は最初から最大の長さで出る
	if (m_info.growSpeed <= 0.0f) return m_info.length;

	//判定が出てから何フレーム経ったか(判定が出る前は0にする)
	const float elapsed = (std::max)(0.0f, m_frame - static_cast<float>(m_info.hitStartFrame));
	//伸びた長さ(最大の長さを超えないようにする)
	return (std::min)(elapsed * m_info.growSpeed, m_info.length);
}

bool EnemyStateAttack::IsHitActive() const
{
	return m_info.hitStartFrame <= m_frame && m_frame < m_info.hitEndFrame;
}

Vector3 EnemyStateAttack::CalcAimDir() const
{
	const Vector3& forward = m_enemy.GetForward();
	const Character* target = m_enemy.GetTarget();
	if (!target) return forward;

	//Character::GetAttackPosと同じ求め方で口の位置を出す
	Vector3 mouthPos = m_enemy.GetPos() + forward * m_enemy.GetHitForwardOffset() + Vector3(0.0f, m_enemy.GetHitHeight(), 0.0f);
	//浮いているので水平に吐くと頭の上を通り過ぎる。体の中心を狙って斜め下に吐く
	Vector3 toTarget = target->GetCenterPos() - mouthPos;
	if (toTarget.SqMagnitude() <= 0.0f) return forward;
	return toTarget.Normalize();
}