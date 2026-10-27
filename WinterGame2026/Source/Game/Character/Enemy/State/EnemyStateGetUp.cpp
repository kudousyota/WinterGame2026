#include "EnemyStateGetUp.h"
#include "EnemyStateIdle.h"

namespace
{
	//起き上がりの再生速度
	constexpr float kGetUpAnimSpeed = 2.0f;
	//浮き上がりの速さ(残りの高さに掛ける割合。大きいほど速い)
	constexpr float kRiseRate = 0.08f;
	//目標の高さにこれ以上近づいたら着いたことにする
	constexpr float kArriveDistance = 1.0f;
}

void EnemyStateGetUp::Enter()
{
	m_enemy.StopMove();
	m_enemy.GetAnim().ChangeAnim(m_enemy.GetGetUpAnimName(), false, kGetUpAnimSpeed);
}

void EnemyStateGetUp::Update()
{
	m_enemy.StopMove();
	
	//起き上がりのアニメーションが終わるまでまつ
	if (!m_isRising)
	{
		if (!m_enemy.GetAnim().GetAnimEndFlag())return;
		//浮き上がり始める
		m_isRising = true;
		m_tagetY = m_enemy.GetPos().y + m_enemy.GetHoverHeight();
		m_enemy.GetAnim().ChangeAnim(m_enemy.GetIdleAnimName(),true);
	}

	float diff = m_tagetY - m_enemy.GetPos().y;
	//着いたら高さを合わせて待機
	if (diff <= kArriveDistance)
	{
		Vector3 pos = m_enemy.GetPos();
		pos.y = m_tagetY;
		m_enemy.SetPosition(pos);
		m_enemy.SetVelocity(Vector3(0.0f, 0.0f, 0.0f));
		ChangeState<EnemyStateIdle>();
		return;
	}
	m_enemy.SetVelocity(Vector3(0.0f, diff * kRiseRate, 0.0f));
}
