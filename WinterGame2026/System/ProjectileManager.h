#pragma once
#include <vector>
#include <memory>

class Projectile;
class ProjectileManager
{
public:
	//シングルトン取得
	static ProjectileManager& Instance();

	//生成したProjectileを追加
	Projectile* Add(std::unique_ptr<Projectile>projectile);

	void Update();
	void Draw();
private:

	ProjectileManager() = default;
	~ProjectileManager() = default;

	//コピー禁止
	ProjectileManager(const ProjectileManager&) = delete;
	ProjectileManager& operator=(const ProjectileManager&) = delete;
	std::vector<std::unique_ptr<Projectile>>m_pProjectiles;
};

