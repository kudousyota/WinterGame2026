#pragma once
#include <memory>
#include "../Game/Character/Player/Player.h"
#include "../Game/Camera.h"
#include "../Game/Stage.h"
#include "../Game/Character/Enemy/EnemyManager.h"
#include "../System/CollisionManager.h"
#include "../System/ProjectileManager.h"
#include "../System/DebugTool.h"
#include "../System/LockOnManager.h"
#include "Scene.h"

class GameScene : public Scene
{
public:
	GameScene(SceneController& controller);
	~GameScene();

	void Init() override;
	void Update() override;
	void Draw() override;
private:
	void DrawGrid();
	//申請された攻撃をまとめて判定し、当たった相手にダメージを与える
	void ProcessAttacks();
	void ProcessShots();
	//飛び道具の当たり判定を行い、当たった相手にダメージを与える
	void ProcessProjectileHits();
	//攻撃が当たったときの処理
	void ApplyAttackHit(Character& attacker, const AttackRequest& request, const CollisionManager::AttackHit& hit);
	//ロックオンの入力を処理して、狙う敵を更新する
	void UpdateLockOn();
	//ロックオンで敵を探す基準の向き(スティックの方向、倒していなければカメラの正面)
	Vector3 GetLockOnBaseDir() const;
	//プレイヤーが攻撃で向く相手(ロックオン中はその敵、そうでなければ入力方向で一番狙いやすい敵)
	std::shared_ptr<const Character> GetAttackTarget() const;
	//敵の一覧(その場で使うだけの処理用。フレームをまたいで持たない)
	std::vector<Enemy*> GetEnemyList() const;
	//敵の一覧(ロックオンなど、weak_ptrで敵を覚えておく処理用)
	std::vector<std::shared_ptr<Character>> GetEnemies() const;
	//プレイヤーと全部の敵(攻撃の判定など、その場で使うだけの処理用)
	std::vector<Character*> GetAllCharacters() const;

	//敵を出す(groundPos:地面の位置。浮いている敵はその分だけ上に置く)
	void SpawnEnemy(const std::shared_ptr<Enemy>& enemy, const Vector3& groundPos);
	//死亡演出が終わって消えた敵を、当たり判定から外してから消す
	void RemoveVanishedEnemies();
	//勝ち負けが決まっていたら、少し待ってからクリア・ゲームオーバーに切り替える
	void CheckGameEnd();


#ifdef _DEBUG
	//デバッグ表示するキャラクターの一覧
	std::vector<DebugTool::Target> GetDebugTargets()const;
#endif // _DEBUG

private:
	int m_frameCount;
	//勝ち負けが決まってから経ったフレーム数
	int m_gameEndFrame = 0;

	std::shared_ptr<Player> m_pPlayer;
	std::shared_ptr<Camera>m_pCamera;
	std::shared_ptr<Stage> m_pStage;

	//キャラクター同士の当たり判定(このシーンの中だけで使う)
	CollisionManager m_collision;
	//飛び道具(シーンと一緒に消えるので、次のシーンに弾が残らない)
	ProjectileManager m_projectiles;
	//ロックオン(このシーンの中だけで使う)
	LockOnManager m_lockOn;
	//敵(敵の持ち主はここだけ)
	EnemyManager m_enemyManager;

#ifdef _DEBUG
	//デバッグ機能(一時停止・表示切り替え・CSVの読み直し)
	DebugTool m_debug;
#endif // _DEBUG
};

