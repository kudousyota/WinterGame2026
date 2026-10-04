#pragma once
#include "../System/Projectile.h"
#include "../DataLoader/AttackData.h"
//ブレス
//撃った敵が毎フレーム位置と向きを更新し、攻撃が終わったらKillする
class Breath : public Projectile
{
public:
	Breath(const Vector3& pos, const Vector3& forward, float speed, const AttackData& attack);

	void Update(float timeScale)override;
	void Draw()override;
};

