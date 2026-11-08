#pragma once
#include "../System/Projectile.h"
#include "../DataLoader/AttackData.h"

//足元から広がるリング状の衝撃波
//一度当たったら判定は止めるが、見た目は最後まで広がる
class Shockwave : public Projectile
{
public:
	/// <summary>
	///
	/// </summary>
	/// <param name="center">中心。撃った敵の足元</param>
	/// <param name="speed">1フレームに広がる量</param>
	/// <param name="attack">攻撃データ</param>
	Shockwave(const Vector3& center, float speed, const AttackData& attack);

	void Update(float timeScale)override;
	//見た目はあとでエフェクトにするので今は何も描かない
	void Draw()override;

	//リングの帯で当たりを調べる
	std::vector<CollisionManager::AttackHit> CheckHits(CollisionManager& collision)const override;
	//当たっても消えずに、判定だけ止める
	void OnHitTarget(bool isJustDodge)override;

#ifdef _DEBUG
	//今の半径の円を描く
	void DrawDebug()const override;
#endif

private:
	//今の半径
	float m_radius = 0.0f;
	//前のフレームの半径
	float m_prevRadius = 0.0f;
	//もう当たったか。当たった後は判定しない
	bool m_hasHit = false;
};
