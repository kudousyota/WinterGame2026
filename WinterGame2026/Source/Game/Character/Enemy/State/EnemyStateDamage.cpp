#include "EnemyStateDamage.h"
#include "EnemyStateIdle.h"
#include <algorithm>

namespace
{
	//のけぞりのアニメーションのこの割合までで下がりきる(当たった直後に勢いよく下がる)
	constexpr float kSlideEndRate = 0.4f;
}

EnemyStateDamage::EnemyStateDamage(Enemy& enemy, float distance) :
	EnemyStateBase(enemy),
	m_distance(distance)
{
}

void EnemyStateDamage::Enter()
{
	//のけぞり中はその場で止まる
	m_enemy.StopMove();
	//攻撃してきた相手の方を向いてのけぞる
	m_enemy.FaceTarget();
	//背中側へ下がる
	m_flyDir = m_enemy.GetForward() * -1.0f;
	m_enemy.GetAnim().ChangeAnim(m_enemy.GetDamageAnimName(), false);
}

void EnemyStateDamage::Update()
{
	m_enemy.StopMove();

	//吹っ飛ぶ攻撃なら、アニメーションの進み具合に合わせて後ろへ下がる
	if (m_distance > 0.0f)
	{
		float t = std::clamp(m_enemy.GetAnim().GetAnimRate() / kSlideEndRate, 0.0f, 1.0f);
		Vector3 delta = m_flyDir * ((t - m_prevT) * m_distance);
		m_enemy.SetPosition(m_enemy.GetPos() + delta);
		m_prevT = t;
	}

	//のけぞりのアニメーションが終わったら待機に戻る
	if (m_enemy.GetAnim().GetAnimEndFlag())
	{
		ChangeState<EnemyStateIdle>();
	}
}