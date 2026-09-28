#include "ProjectileManager.h"
#include "../System/Projectile.h"
#include "Timer.h"
#include <algorithm>

//シングルトンインスタンスを取得//シングルトンがいっぱいあるのよくないから設計悩み
ProjectileManager& ProjectileManager::Instance()
{
	static ProjectileManager manager;
	return manager;
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
	//敵の飛び道具なので敵と同じ時間倍率で動かす(ウィッチタイム中は遅くなる)
	float timeScale = Timer::Instance().GetEnemyTimeScale();
	//登録した全ての飛び道具を更新する
	for (auto& projectile : m_pProjectiles)
	{
		projectile->Update(timeScale);
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
		projectile->Draw();
	}

}