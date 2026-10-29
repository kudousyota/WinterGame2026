#include "EnemyManager.h"

void EnemyManager::Add(const std::shared_ptr<Enemy>& enemy)
{
	m_enemies.push_back(enemy);
}

void EnemyManager::Update()
{
	for (const auto& enemy : m_enemies)
	{
		enemy->Update();
	}
}

void EnemyManager::LateUpdate()
{
	for (const auto& enemy : m_enemies)
	{
		enemy->LateUpdate();
	}
}

void EnemyManager::Draw()
{
	for (const auto& enemy : m_enemies)
	{
		enemy->Draw();
	}
}

void EnemyManager::RemoveVanished()
{
	//リストから外すと、ほかに持っている人がいなければここで解放される
	//weak_ptrで見ている側(ロックオンなど)は、lock()が空を返すようになる
	std::erase_if(m_enemies, [](const std::shared_ptr<Enemy>& enemy)
		{
			return enemy->IsVanished();
		});
}
