#include "Enemy.h"
#include <cfloat>

Enemy::Enemy()
{
}

Enemy::~Enemy()
{
}

void Enemy::Init()
{

}

void Enemy::Update()
{

}

void Enemy::Draw()
{

}

float Enemy::GetTargetDistance() const
{
	if (!m_pTarget) return FLT_MAX;

	Vector3 toTarget = m_pTarget->GetPos() - m_pos;
	toTarget.y = 0.0f;
	return toTarget.Magnitude();
}

void Enemy::MoveToTarget(float speedScale)
{
	if (!m_pTarget) return;

	Vector3 toTarget = m_pTarget->GetPos() - m_pos;
	toTarget.y = 0.0f;
	//ほぼ重なっている時は向きが決まらないので止まる
	if (toTarget.SqMagnitude() <= 0.0f)
	{
		StopMove();
		return;
	}

	Vector3 move = toTarget.Normalize() * (m_speed * speedScale);
	m_velocity = Vector3(move.x, m_velocity.y, move.z);
}
