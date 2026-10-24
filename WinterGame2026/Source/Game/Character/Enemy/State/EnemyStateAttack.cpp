#include "EnemyStateAttack.h"
#include "EnemyStateRetreat.h"
#include "../../../../DataLoader/DataManager.h"
#include "../../../../Effect/EffectManager.h"
#include <cstdio>
#include <cmath>

namespace
{
	//攻撃判定を出す位置(口の位置。デバッグ表示を見ながら調整する)
	constexpr float kHitForwardOffset = 50.0f;
	constexpr float kHitHeight = 120.0f;
	//予兆エフェクトの拡大率(Effekseerの1単位を何cmにするか。見た目を見ながら調整する)
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
	m_enemy.GetAnim().ChangeAnim(m_info.animName, false);
	m_attackId = m_enemy.IssueAttackId();
	m_nextHitFrame = static_cast<float>(m_info.hitStartFrame + m_info.hitInterval);

	//予兆が始まった瞬間に狙いを決める(この後プレイヤーが動けば避けられる)
	m_aimDir = CalcAimDir();
	//予兆の始まり
	m_enemy.SetWarning(true);

	//予兆エフェクトを口の位置に出す(攻撃中は敵が止まっているので位置は最初に決めるだけでいい)
	const Vector3& forward = m_enemy.GetForward();
	const Vector3 mouthPos = m_enemy.GetPos() + forward * kHitForwardOffset + Vector3(0.0f, kHitHeight, 0.0f);
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
		request.forwardOffset = kHitForwardOffset;
		request.height = kHitHeight;
		//lengthが0なら今まで通りの球、0より大きければ狙った方向へ伸びるカプセルになる
		request.dir = m_aimDir;
		request.length = m_info.length;
		m_enemy.RequestAttack(request);
	}

	m_frame += m_enemy.GetTimeScale();
}

void EnemyStateAttack::Exit()
{
	//攻撃の途中でのけぞったときも予兆が出っぱなしにならないようにする
	m_enemy.SetWarning(false);
	StopTelegraph();
}

void EnemyStateAttack::StopTelegraph()
{
	if (m_telegraphHandle == -1) return;
	//再生が終わって管理リストから消えていても、StopEffectは見つからなければ何もしない
	EffectManager::Instns().StopEffect(m_telegraphHandle);
	m_telegraphHandle = -1;
}

#ifdef _DEBUG
std::string EnemyStateAttack::GetDebugInfo() const
{
	char buf[128];
	sprintf_s(buf, "%s  Frame:%.1f/%d  Hit:%s  Id:%d",
		m_info.animName.c_str(), m_frame, m_info.totalFrame, IsHitActive() ? "ON" : "OFF", m_attackId);
	return buf;
}
#endif

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
	Vector3 mouthPos = m_enemy.GetPos() + forward * kHitForwardOffset + Vector3(0.0f, kHitHeight, 0.0f);
	//浮いているので水平に吐くと頭の上を通り過ぎる。体の中心を狙って斜め下に吐く
	Vector3 toTarget = target->GetCenterPos() - mouthPos;
	if (toTarget.SqMagnitude() <= 0.0f) return forward;
	return toTarget.Normalize();
}