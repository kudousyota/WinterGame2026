#include "Bullet.h"
#include "../Effect/EffectManager.h"

namespace
{
	//弾の寿命
	constexpr float kBulletLifeFrame = 60.0f;
	//魔法弾エフェクトの拡大率(Effekseerの1単位を何cmにするか)
	constexpr float kBulletEffectScale = 8.0f;
}

Bullet::Bullet(const Vector3& pos, const Vector3& forward, float speed, const AttackData& attack):
	Projectile(pos, forward, speed, attack,kBulletLifeFrame)
{
	//見た目はエフェクトで出す。エフェクトは動かないので、毎フレーム弾の位置に合わせる
	m_effectHandle = EffectManager::Instns().PlayEffect(EffectType::MagicBullet, m_pos, Vector3(), kBulletEffectScale);
}

Bullet::~Bullet()
{
	//すぐ消すと尾も消えるので、フェードさせる(尾の粒は寿命まで残る)
	EffectManager::Instns().FadeOutEffect(m_effectHandle);
}

void Bullet::Update(float timeScale)
{

	//寿命
	UpdateLife(timeScale);
	m_prevPos = m_pos;
	m_pos += m_forward * m_speed * timeScale;

	//動いた先にエフェクトを持っていく(通った跡に尾が残る)
	EffectManager::Instns().SetEffectPos(m_effectHandle, m_pos);
}

void Bullet::Draw()
{
	//見た目はEffectManagerが描くので、ここでは何もしない
}
