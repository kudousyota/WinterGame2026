#pragma once
#include <memory>
#include <vector>
#include "Enemy.h"

//敵をまとめて持つ
//敵の持ち主はここだけにする
//ほかのクラスは敵をshared_ptrでメンバに持たない(持つとリストから外しても消えなくなる)
//フレームをまたいで覚えておくときはweak_ptr、その場で使うだけなら生ポインタでよい
class EnemyManager
{
public:
	//敵をリストに加える//Initと位置の設定は呼ぶ側で先に行う
	void Add(const std::shared_ptr<Enemy>& enemy);

	void Update();
	//当たり判定の押し出しが終わった後に呼ぶ
	void LateUpdate();
	void Draw();

	//死亡演出が終わって消えた敵をリストから外す
	void RemoveVanished();
	//全部の敵をリストから外す
	void Clear();

	//攻撃していいか
	bool TryTakeAttackToken();
	//攻撃したら攻撃枠を返す
	void ReleaseAttackToken();

	const std::vector<std::shared_ptr<Enemy>>& GetEnemies()const { return m_enemies; }

private:
	//今攻撃している敵の数
	int m_attackerCount = 0;
	std::vector<std::shared_ptr<Enemy>> m_enemies;
};
