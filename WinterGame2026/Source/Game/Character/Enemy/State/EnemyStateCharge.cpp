#include "EnemyStateCharge.h"
#include "../DataLoader/DataManager.h"

EnemyStateCharge::EnemyStateCharge(Enemy& enemy) : 
	EnemyStateBase(enemy),
	m_info(DataManager::GetInstance().GetAttackTable().GetInfo(enemy.GetAttackType()))
{
}

void EnemyStateCharge::Enter()
{
	m_enemy.StopMove();
	m_enemy.FaceTarget();
	m_enemy.GetAnim().ChangeAnim(m_info.animName,false);
	m_attackId = m_enemy.IssueAttackId();
	m_enemy.SetWarning(true);
}

void EnemyStateCharge::Update()
{
}

void EnemyStateCharge::Exit()
{
}
