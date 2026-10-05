#include "EnemyStateChase.h"
#include "EnemyStateIdle.h"
#include "EnemyStateAttack.h"

void EnemyStateChase::Enter()
{
	m_enemy.GetAnim().ChangeAnim(m_enemy.GetMoveAnimName(), true);
}

void EnemyStateChase::Update()
{
	float distance = m_enemy.GetTargetDistance();

	//攻撃範囲に入ったら攻撃する
	if (distance <= m_enemy.GetAttackRange())
	{
		ChangeState<EnemyStateAttack>();
		return;
	}
	//見失ったら待機に戻る
	if (distance > m_enemy.GetSearchRange())
	{
		ChangeState<EnemyStateIdle>();
		return;
	}

	m_enemy.FaceTarget();
	m_enemy.MoveToTarget(1.0f);
}
