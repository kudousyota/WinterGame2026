#include "Projectile.h"

Projectile::Projectile(const Vector3& pos, const Vector3& forward, float speed, const AttackData& attack, float maxLifeFrame) :
	m_pos(pos.x, pos.y, pos.z),
	m_prevPos(pos.x, pos.y, pos.z),
	m_forward(forward.x, forward.y, forward.z),
	m_speed(speed),
	m_lifeFrame(0.0f),
	m_maxLifeFrame(maxLifeFrame),
	m_isDead(false),
	m_attackData(attack)
{
}

void Projectile::UpdateLife(float timeScale)
{
	m_lifeFrame += timeScale;
	//寿命なしのときは消さない
	if (m_maxLifeFrame > 0.0f && m_lifeFrame >= m_maxLifeFrame)
	{
		Kill();
	}
}
