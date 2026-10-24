#include "EnemyStateIdle.h"
#include "EnemyStateChase.h"
#include "EnemyStateAttack.h"
#include "EnemyStateRetreat.h"

namespace
{
	//次の行動を決めるまで待つフレーム数
	constexpr float kWaitFrame = 60.0f;
}

void EnemyStateIdle::Enter()
{
	m_enemy.StopMove();
	m_enemy.GetAnim().ChangeAnim(m_enemy.GetIdleAnimName(), true);
}

void EnemyStateIdle::Update()
{
	m_enemy.StopMove();

	m_frame += m_enemy.GetTimeScale();
	if (m_frame < kWaitFrame) return;

	//倒れたプレイヤーには何もしない
	const Character* target = m_enemy.GetTarget();
	if (!target || target->IsDead()) return;

	float distance = m_enemy.GetTargetDistance();
	//近すぎたら離れる(近距離の攻撃は持っていない)
	if (distance < m_enemy.GetTooCloseDistance())
	{
		ChangeState<EnemyStateRetreat>();
		return;
	}
	//攻撃範囲にいれば攻撃する
	if (distance <= m_enemy.GetAttackRange())
	{
		ChangeState<EnemyStateAttack>();
		return;
	}
	//索敵範囲にいれば追いかける
	if (distance <= m_enemy.GetSearchRange())
	{
		ChangeState<EnemyStateChase>();
		return;
	}
}
