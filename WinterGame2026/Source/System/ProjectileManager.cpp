#include "ProjectileManager.h"
#include "../System/Projectile.h"
#include "Timer.h"
#include <algorithm>

ProjectileManager::ProjectileManager()
{
}

ProjectileManager::~ProjectileManager()
{
}
//飛び道具を登録
Projectile* ProjectileManager::Add(std::unique_ptr<Projectile> projectile)
{
	//呼び出しもとで操作できるようにポインタを保持
	Projectile* ptr = projectile.get();
	//管理するリストに追加
	m_pProjectiles.push_back(std::move(projectile));
	return ptr;
}
//登録された飛び道具の更新
void ProjectileManager::Update()
{
	//撃った側の時間倍率で動かす
	float gameTimeScale = Timer::Instance().GetTimeScale();
	float enemyTimeScale =  gameTimeScale *Timer::Instance().GetEnemyTimeScale();
	//登録した全ての飛び道具を更新する
	for (auto& projectile : m_pProjectiles)
	{
		bool isEnemy = projectile->GetAttackData().GetAttacker() == CharacterType::Enemy;
		if (isEnemy)
		{
			projectile->Update(enemyTimeScale);
		}
		else
		{
			projectile->Update(gameTimeScale);
		}
	}
	//死んだ飛び道具を管理から消す
	m_pProjectiles.erase(
		//IsDead()がtrueの要素をコンテナの末尾に移動//ここで削除対象を集める
		std::remove_if(m_pProjectiles.begin(), m_pProjectiles.end(),
			//消す対象なのかを判断する
			[](const std::unique_ptr<Projectile>& projectile)
			{
				//死んでいたら削除する
				return projectile->IsDead();
			}),
		//remove_ifで集められた要らない物をここで消す
		m_pProjectiles.end()
	);
}
//登録した飛び道具の描画
void ProjectileManager::Draw()
{
	for (auto& projectile : m_pProjectiles)
	{
		//当たって消えた飛び道具は次のUpdateまで残っているので描かない
		if (projectile->IsDead()) continue;
		projectile->Draw();
	}

}