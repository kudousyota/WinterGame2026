#pragma once
#include "../System/Projectile.h"
#include "../DataLoader/AttackData.h"
//ブレス
//撃った敵が毎フレーム位置と向きを更新し、攻撃が終わったらKillする
class Breath : public Projectile
{
public:
	/// <summary>
	/// 
	/// </summary>
	/// <param name="pos">位置</param>
	/// <param name="forward">前方向</param>
	/// <param name="speed">速度</param>
	/// <param name="attack">攻撃データ</param>
	Breath(const Vector3& pos, const Vector3& forward, float speed, const AttackData& attack);

	void Update(float timeScale)override;
	void Draw()override;
};

