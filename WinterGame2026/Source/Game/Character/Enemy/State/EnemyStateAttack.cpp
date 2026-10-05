#include "EnemyStateAttack.h"
#include "EnemyStateIdle.h"
#include "../../../../DataLoader/DataManager.h"

namespace
{
	//攻撃判定を出す位置(足元から前方への距離と高さ)
	constexpr float kHitForwardOffset = 100.0f;
	constexpr float kHitHeight = 100.0f;
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
	//攻撃を始めた時の方向に攻撃する(途中で向きを変えないので横に避けられる)
	m_enemy.FaceTarget();
	m_enemy.GetAnim().ChangeAnim(m_info.animName, false);
	m_attackId = m_enemy.IssueAttackId();
}

void EnemyStateAttack::Update()
{
	m_enemy.StopMove();

	//攻撃が終わったら待機(攻撃の後の隙)にする
	if (m_frame >= m_info.totalFrame)
	{
		ChangeState<EnemyStateIdle>();
		return;
	}

	if (IsHitActive())
	{
		//攻撃を申請するだけ(判定はGameSceneが全員の移動が終わった後に行う)
		AttackRequest request{ AttackData(CharacterType::Enemy, m_info.type, m_info.damage, m_info.radius) };
		request.attackId = m_attackId;
		request.forwardOffset = kHitForwardOffset;
		request.height = kHitHeight;
		m_enemy.RequestAttack(request);
	}

	m_frame += m_enemy.GetTimeScale();
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
