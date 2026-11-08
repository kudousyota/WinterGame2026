#include "EnemyStateJumpAttack.h"
#include "EnemyStateRetreat.h"
#include "../../../../DataLoader/DataManager.h"
#include "../../../../Effect/EffectManager.h"
#include <cstdio>
#include <cmath>

namespace
{
	//予兆エフェクトの拡大率
	constexpr float kTelegraphScale = 20.0f;

	//着地で出す衝撃波
	//1フレームに広がる量
	constexpr float kShockwaveSpeed = 25.0f;
	//ダメージ
	//着地の直撃よりは弱くする
	constexpr int kShockwaveDamage = 10;
}

EnemyStateJumpAttack::EnemyStateJumpAttack(Enemy& enemy) :
	EnemyStateBase(enemy),
	m_info(DataManager::GetInstance().GetAttackTable().GetInfo(enemy.GetAttackType()))
{
}

void EnemyStateJumpAttack::Enter()
{
	m_enemy.StopMove();
	m_enemy.FaceTarget();
	//溜めがある攻撃は、最初はゆっくり再生して力を溜めているように見せる
	const float animScale = (m_info.windupFrame > 0) ? m_info.windupAnimScale : 1.0f;
	m_enemy.GetAnim().ChangeAnim(m_info.animName, false, animScale);
	m_attackId = m_enemy.IssueAttackId();

	//着地するまでが予兆
	m_enemy.SetWarning(true);

	//予兆エフェクトを出す
	const Vector3& forward = m_enemy.GetForward();
	const Vector3 effectPos = m_enemy.GetPos() + forward * m_enemy.GetHitForwardOffset() + Vector3(0.0f, m_enemy.GetHitHeight(), 0.0f);
	Vector3 rot;
	rot.y = atan2f(-forward.x, -forward.z);
	m_telegraphHandle = EffectManager::Instns().PlayEffect(EffectType::AttackTelegraph, effectPos, rot, kTelegraphScale);
}

void EnemyStateJumpAttack::Update()
{
	//攻撃が終わったらプレイヤーから離れる
	if (m_frame >= m_info.totalFrame)
	{
		ChangeState<EnemyStateRetreat>();
		return;
	}

	//溜めが終わったら普通の速さに戻す
	if (m_info.windupFrame > 0 && m_frame >= m_info.windupFrame)
	{
		m_enemy.GetAnim().SetAnimTimeScale(1.0f);
	}

	//その場で跳ぶので動かない
	m_enemy.StopMove();

	if (m_frame < m_info.moveStartFrame)
	{
		//溜めている間はプレイヤーの方を向き続ける
		m_enemy.FaceTarget();
	}
	else
	{
		//跳んだら予兆エフェクトは消す
		StopTelegraph();
	}

	//判定が出るまでが予兆
	m_enemy.SetWarning(m_frame < m_info.hitStartFrame);

	if (IsHitActive())
	{
		//着地の直撃
		//足元を中心にした球で判定する
		AttackRequest request{ AttackData(CharacterType::Enemy, m_info.type, m_info.damage, m_info.radius) };
		request.attackId = m_attackId;
		m_enemy.RequestAttack(request);

		//判定が出た瞬間に衝撃波を1回だけ出す
		if (!m_hasShockwave)
		{
			ShotRequest shot{ AttackData(CharacterType::Enemy, m_info.type, kShockwaveDamage, 0.0f) };
			shot.kind = ShotKind::Shockwave;
			shot.speed = kShockwaveSpeed;
			//forwardOffsetとheightは0のままにして、足元から広げる
			m_enemy.RequestShot(shot);
			m_hasShockwave = true;
		}
	}

	m_frame += m_enemy.GetTimeScale();
}

void EnemyStateJumpAttack::Exit()
{
	m_enemy.SetWarning(false);
	StopTelegraph();
	//攻撃が終わったので、ほかの敵が攻撃できるようにする
	m_enemy.ReleaseAttackToken();
}

bool EnemyStateJumpAttack::IsHitActive() const
{
	return m_info.hitStartFrame <= m_frame && m_frame < m_info.hitEndFrame;
}

void EnemyStateJumpAttack::StopTelegraph()
{
	if (m_telegraphHandle == -1) return;
	EffectManager::Instns().StopEffect(m_telegraphHandle);
	m_telegraphHandle = -1;
}

#ifdef _DEBUG
std::string EnemyStateJumpAttack::GetDebugInfo() const
{
	const char* phase = "Windup";
	if (m_frame >= m_info.moveEndFrame) phase = "Landed";
	else if (m_frame >= m_info.moveStartFrame) phase = "Air";

	char buf[160];
	sprintf_s(buf, "%s  Frame:%.1f/%d  Phase:%s  Hit:%s  Id:%d",
		m_info.animName.c_str(), m_frame, m_info.totalFrame, phase, IsHitActive() ? "ON" : "OFF", m_attackId);
	return buf;
}
#endif
