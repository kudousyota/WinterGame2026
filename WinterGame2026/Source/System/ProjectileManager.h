#pragma once
#include <vector>
#include <memory>

class Projectile;
//飛び道具を管理する
//GameSceneが所有する
class ProjectileManager
{
public:
	//Projectileは前方宣言なので、コンストラクタ・デストラクタの定義はcpp側に置く
	ProjectileManager();
	~ProjectileManager();

	//コピー禁止
	ProjectileManager(const ProjectileManager&) = delete;
	ProjectileManager& operator=(const ProjectileManager&) = delete;

	//生成したProjectileを追加
	Projectile* Add(std::unique_ptr<Projectile>projectile);

	void Update();
	void Draw();
private:
	std::vector<std::unique_ptr<Projectile>>m_pProjectiles;
};

