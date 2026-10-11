#pragma once
#include "../System/Projectile.h"
#include "../DataLoader/AttackData.h"
class Bullet: public Projectile
{
public:
	/// <summary>
	/// 
	/// </summary>
	/// <param name="pos">位置</param>
	/// <param name="forward">前方向</param>
	/// <param name="speed">速度</param>
	/// <param name="attack">攻撃データ</param>
	Bullet(const Vector3& pos, const Vector3& forward, float speed, const AttackData& attack);
	void Update(float timeScale)override;
	void Draw()override;
};

