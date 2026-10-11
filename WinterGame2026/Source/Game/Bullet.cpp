#include "Bullet.h"

namespace
{
	//弾の寿命
	constexpr float kBulletLifeFrame = 60.0f;
}

Bullet::Bullet(const Vector3& pos, const Vector3& forward, float speed, const AttackData& attack):
	Projectile(pos, forward, speed, attack,kBulletLifeFrame)
{

}

void Bullet::Update(float timeScale)
{

	//寿命
	UpdateLife(timeScale);
	m_prevPos = m_pos;
	m_pos += m_forward * m_speed * timeScale;
	
}

void Bullet::Draw()
{
	//弾の描画
	DrawSphere3D(m_pos, m_attackData.GetRadius(), 16, 0xffffff, 0xffffff, true);
}
