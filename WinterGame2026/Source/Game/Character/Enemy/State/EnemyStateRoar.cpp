#include "EnemyStateRoar.h"
#include "EnemyStateIdle.h"

void EnemyStateRoar::Enter()
{
	m_enemy.StopMove();
	//1回だけ流す//ループさせると最後で姿勢が飛ぶ
	m_enemy.GetAnim().ChangeAnim(m_enemy.GetRoarAnimName(), false);
}

void EnemyStateRoar::Update()
{
	m_enemy.StopMove();

	//叫び終わったら待機に戻る
	if (m_enemy.GetAnim().GetAnimEndFlag())
	{
		ChangeState<EnemyStateIdle>();
	}
}
