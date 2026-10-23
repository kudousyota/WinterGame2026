#include "EnemyStateDamage.h"
#include "EnemyStateIdle.h"

void EnemyStateDamage::Enter()
{
	//のけぞり中はその場で止まる
	m_enemy.StopMove();
	//攻撃してきた相手の方を向いてのけぞる
	m_enemy.FaceTarget();
	m_enemy.GetAnim().ChangeAnim(m_enemy.GetDamageAnimName(), false);
}

void EnemyStateDamage::Update()
{
	m_enemy.StopMove();

	//のけぞりのアニメーションが終わったら待機に戻る
	if (m_enemy.GetAnim().GetAnimEndFlag())
	{
		ChangeState<EnemyStateIdle>();
	}
}
