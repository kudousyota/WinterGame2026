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

	//攻撃できる距離に入ったら攻撃する
	if (m_enemy.CanStartAttack(distance))
	{
		//ほかの敵が攻撃中なら、横に回って順番を待つ
		if (m_enemy.TryTakeAttackToken())
		{
			//攻撃ステートは作られたときに攻撃のデータを読むので、先に攻撃を決める
			m_enemy.DecideAttack();
			//どのステートで攻撃するかは敵ごとに決める
			m_enemy.ChangeState(m_enemy.CreateAttackState());
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
