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
	//消えるときに見た目のエフェクトも消す
	~Bullet()override;
	void Update(float timeScale)override;
	void Draw()override;
private:
	//見た目の魔法弾エフェクト
	int m_effectHandle = -1;
};

