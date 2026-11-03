#include "EnemyStateChase.h"
#include "EnemyStateIdle.h"
#include "EnemyStateAttack.h"
#include "EnemyStateObserve.h"

void EnemyStateChase::Enter()
{
	m_enemy.GetAnim().ChangeAnim(m_enemy.GetMoveAnimName(), true);
}

void EnemyStateChase::Update()
{
	float distance = m_enemy.GetTargetDistance();

	//倒れたプレイヤーは追いかけない
	const Character* target = m_enemy.GetTarget();
	if (!target || target->IsDead())
	{
		ChangeState<EnemyStateIdle>();
		return;
	}

	//攻撃範囲に入ったら攻撃する
	if (distance <= m_enemy.GetAttackRange())
	{
		//ほかの敵が攻撃中なら、横に回って順番を待つ
		if (m_enemy.TryTakeAttackToken())
		{
			ChangeState<EnemyStateAttack>();
		}
		else
		{
			ChangeState<EnemyStateObserve>();
		}
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
