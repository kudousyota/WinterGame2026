#include "Breath.h"
#include "DxLib.h"

namespace
{
	//ブレスの長さ
	constexpr float kBreathRange = 200.0f;
}

Breath::Breath(const Vector3& pos, const Vector3& forward, float speed, const AttackData& attack) :
	//親に渡す
	//寿命は撃った敵がKillで管理するので寿命なし
	Projectile(pos, forward, speed, attack)
{

}

void Breath::Update(float timeScale)
{
	//寿命
	UpdateLife(timeScale);
}

void Breath::Draw()
{
#ifdef _DEBUG
	//当たり範囲の表示
	Vector3 endpos = m_pos + m_forward * kBreathRange;
	DrawCapsule3D(m_pos, endpos, m_attackData.GetRadius(), 4, 0xffffff, 0xffffff, false);
#endif // _DEBUG
}
