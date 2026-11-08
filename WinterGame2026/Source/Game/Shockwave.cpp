#include "Shockwave.h"
#include "DxLib.h"
#include <cmath>

namespace
{
	//ここまで広がったら消える
	constexpr float kMaxRadius = 1500.0f;
	//波の太さの半分
	constexpr float kHalfWidth = 40.0f;
	//足元がこの高さより上なら当たらない。ジャンプで越えられる
	constexpr float kWaveHeight = 60.0f;

#ifdef _DEBUG
	//デバッグ表示の円を何本の線で描くか
	constexpr int kDebugCircleDiv = 32;
	//まだ当たる波の色
	constexpr unsigned int kDebugActiveColor = 0xff8800;
	//もう当たらない波の色
	constexpr unsigned int kDebugInactiveColor = 0x888888;
#endif
}

Shockwave::Shockwave(const Vector3& center, float speed, const AttackData& attack) :
	//向きは使わない
	//寿命は半径で決めるので寿命なしにする
	Projectile(center, Vector3(), speed, attack)
{
}

void Shockwave::Update(float timeScale)
{
	m_prevRadius = m_radius;
	//ウィッチタイム中はtimeScaleが小さくなるので、広がるのも遅くなる
	m_radius += m_speed * timeScale;

	//最大まで広がったら消える
	if (m_radius >= kMaxRadius)
	{
		Kill();
	}
}

void Shockwave::Draw()
{
}

std::vector<CollisionManager::AttackHit> Shockwave::CheckHits(CollisionManager& collision) const
{
	//一度当たったらもう当たらない
	if (m_hasHit) return {};

	//前のフレームの半径から今の半径までまとめて調べる
	//速く広がっても、間にいる相手をすり抜けないようにする
	return collision.CheckAttackRing(m_attackData, m_pos, m_prevRadius - kHalfWidth, m_radius + kHalfWidth, kWaveHeight);
}

void Shockwave::OnHitTarget(bool isJustDodge)
{
	//弾と違って消えずに、判定だけ止める
	m_hasHit = true;
}

#ifdef _DEBUG
void Shockwave::DrawDebug() const
{
	const unsigned int color = m_hasHit ? kDebugInactiveColor : kDebugActiveColor;
	//円を短い線でつないで描く
	for (int i = 0; i < kDebugCircleDiv; i++)
	{
		const float angle0 = DX_TWO_PI_F * i / kDebugCircleDiv;
		const float angle1 = DX_TWO_PI_F * (i + 1) / kDebugCircleDiv;
		const Vector3 p0 = m_pos + Vector3(cosf(angle0), 0.0f, sinf(angle0)) * m_radius;
		const Vector3 p1 = m_pos + Vector3(cosf(angle1), 0.0f, sinf(angle1)) * m_radius;
		DrawLine3D(p0, p1, color);
	}
}
#endif
