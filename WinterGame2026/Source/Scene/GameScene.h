#pragma once
#include <memory>
#include "../Game/Character/Player/Player.h"
#include "../Game/Camera.h"
#include "../Game/Character/Enemy/Alien/Alien.h"
#include "../System/CollisionManager.h"
#include "../System/ProjectileManager.h"
#include "../System/DebugTool.h"

class GameScene
{
public:
	GameScene();
	~GameScene();

	void Init();
	void Update();
	void Draw();
private:
	void DrawGrid();
	//申請された攻撃をまとめて判定し、当たった相手にダメージを与える
	void ProcessAttacks();
	//攻撃が当たったときの処理
	void ApplyAttackHit(Character& attacker, const AttackRequest& request, const CollisionManager::AttackHit& hit);
#ifdef _DEBUG
	//デバッグ表示するキャラクターの一覧
	std::vector<DebugTool::Target> GetDebugTargets()const;
#endif // _DEBUG

private:
	int m_frameCount;

	std::shared_ptr<Player> m_pPlayer;
	std::shared_ptr<Alien> m_pAlien;
	std::shared_ptr<Camera>m_pCamera;

	//キャラクター同士の当たり判定(このシーンの中だけで使う)
	CollisionManager m_collision;
	//飛び道具(シーンと一緒に消えるので、次のシーンに弾が残らない)
	ProjectileManager m_projectiles;

#ifdef _DEBUG
	//デバッグ機能(一時停止・表示切り替え・CSVの読み直し)
	DebugTool m_debug;
#endif // _DEBUG
};

