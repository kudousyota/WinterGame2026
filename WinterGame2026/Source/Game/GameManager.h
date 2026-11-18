#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Character/Player/Player.h"
#include "Camera.h"
#include "Stage.h"
#include "Gate.h"
#include "Character/Enemy/EnemyManager.h"
#include "../System/CollisionManager.h"
#include "../System/ProjectileManager.h"
#include "../System/BattleSystem.h"
#include "../System/LockOnManager.h"
#include "../System/DebugTool.h"

//本編とチュートリアルで共通のゲームの中身をまとめて持つ
//ステージ・プレイヤー・カメラ・敵・当たり判定・飛び道具・戦闘・ロックオン・ゲート
//どう進めるかと見た目の演出は持たない//それはシーンが決める
class GameManager
{
public:
	GameManager();

	//コピー禁止
	GameManager(const GameManager&) = delete;
	GameManager& operator=(const GameManager&) = delete;

	//ステージ・プレイヤー・カメラを作る
	//maxGateNumはゲートの最大数//ゲートがないなら0
	void Init(int maxGateNum);

	//キャラクターの更新から攻撃判定・カメラまで1フレーム分進める
	//ヒットストップ中は呼ばずにUpdateCameraだけ呼ぶ
	void Update();
	//カメラだけ進める//ヒットストップ中もシェイクは動かす
	void UpdateCamera();

	//このフレームの戦闘で起きたことを受け取る//受け取ったら中身は空になる
	//カメラの揺れはもう出してあるので、シーンは歪みなどの演出に使う
	//キャラクターのポインタはRemoveVanishedEnemiesより前だけ使える
	std::vector<BattleEvent> TakeBattleEvents();

	//死亡演出が終わって消えた敵を、当たり判定から外してから消す
	//シーンが戦闘のイベントを使い終わってから呼ぶ
	void RemoveVanishedEnemies();

	//名前から敵を作って出す
	//groundPosは地面の位置//高さが違う床に合わせ、浮いている敵はその分上に置く
	void SpawnEnemy(const std::string& name, const Vector3& groundPos);

	//戦闘中だけゲートを閉じる
	void UpdateGate(bool shouldClose, const std::vector<SpawnTable::GateInfo>& gates);

	//ステージを描く//ウィッチタイムの色を重ねる前に描くので分けている
	void DrawStage();
	//プレイヤー・敵・飛び道具を描く
	void DrawObjects();
#ifdef _DEBUG
	//当たり判定やロックオン範囲などを、DebugToolの設定に合わせて描く
	void DrawDebug(const DebugTool& debug)const;
	//デバッグ表示するキャラクターの一覧
	std::vector<DebugTool::Target> GetDebugTargets()const;
#endif // _DEBUG

	Player& GetPlayer() { return *m_pPlayer; }
	const Camera& GetCamera()const { return *m_pCamera; }
	//残っている敵の数//消えた敵を外した後に使う
	int GetEnemyCount()const;
	//全部の敵が消えたか
	bool AreAllEnemiesVanished()const;

private:
	//敵の一覧//その場で使うだけの処理用。フレームをまたいで持たない
	std::vector<Enemy*> GetEnemyList()const;
	//敵の一覧//ロックオンなど、weak_ptrで敵を覚えておく処理用
	std::vector<std::shared_ptr<Character>> GetEnemies()const;
	//プレイヤーと全部の敵//攻撃の判定など、その場で使うだけの処理用
	std::vector<Character*> GetAllCharacters()const;

	std::shared_ptr<Stage> m_pStage;
	std::shared_ptr<Player> m_pPlayer;
	std::shared_ptr<Camera> m_pCamera;

	//キャラクター同士の当たり判定
	CollisionManager m_collision;
	//飛び道具
	ProjectileManager m_projectiles;
	//攻撃が当たったあとのルール
	//m_collisionとm_projectilesを借りるので、その2つより後に書く
	//メンバは書いた順に作られる
	BattleSystem m_battle;
	//ロックオン
	LockOnManager m_lockOn;
	//敵の持ち主はここだけ
	EnemyManager m_enemyManager;
	//戦闘中だけ閉じる壁
	Gate m_gate;

	//シーンに渡す戦闘のイベント
	std::vector<BattleEvent> m_battleEvents;
};
