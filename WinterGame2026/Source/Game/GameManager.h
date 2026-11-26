#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Character/Player/Player.h"
#include "Camera.h"
#include "EventCamera.h"
#include "BossCutscene.h"
#include "Stage.h"
#include "Gate.h"
#include "Character/Enemy/EnemyManager.h"
#include "../System/CollisionManager.h"
#include "../System/ProjectileManager.h"
#include "../System/BattleSystem.h"
#include "../System/LockOnManager.h"
#include "../System/DebugTool.h"
#include "../Effect/ScreenDistortion.h"

//本編とチュートリアルで共通のゲームの中身をまとめて持つ
class GameManager
{
public:
	GameManager();

	//コピー禁止
	GameManager(const GameManager&) = delete;
	GameManager& operator=(const GameManager&) = delete;

	//ステージ、プレイヤー、カメラを作る
	//stageTypeは読み込むステージ//maxGateNumはゲートの最大数。ゲートがないなら0
	void Init(Stage::Type stageType, int maxGateNum);

	//1フレーム分進める
	//タイマー、キャラクター、攻撃判定、カメラ、エフェクト、消えた敵の片付け
	//ヒットストップで止まっていたらfalse、ゲームが進んだらtrueを返す
	bool Update();

	//このフレームの戦闘で起きたことを受け取る//受け取ったら中身は空になる
	//カメラの揺れとジャスト回避の歪みはもう出してあるので、シーンはそれ以外の進行や演出に使う
	//消えた敵はUpdateの中で片付け済みなので、シーンではattacker、targetのポインタを使わない
	std::vector<BattleEvent> TakeBattleEvents();

	//名前から敵を作って出す
	//groundPosは地面の位置//高さが違う床に合わせ、浮いている敵はその分上に置く
	void SpawnEnemy(const std::string& name, const Vector3& groundPos);
	//全部の敵をすぐに消す//死亡演出は出さない
	//チュートリアルでステップが変わったときに使う
	void ClearEnemies();

	//演出用のカメラを始める//今のカメラの位置からショットへ寄せる
	//演出中は普段のカメラを動かさないので、右スティックでは回らない
	void StartEventCamera(const EventShot& shot);
	//演出の途中で次のショットへ切り替える
	void SetEventShot(const EventShot& shot);
	//演出用のカメラを終えて普段のカメラへ戻す
	void EndEventCamera();
	//演出用のカメラを使っているか
	bool IsEventCameraActive()const { return m_eventCamera.IsActive(); }
	//今のショットにほぼ着いたか
	bool IsEventShotArrived()const { return m_eventCamera.IsArrived(); }
	//普段のカメラのポーズ//演出中は動かさないので、演出が始まる前のポーズのまま
	CameraPose GetGameplayCameraPose()const { return m_pCamera->GetPose(); }
	//画面を揺らす//演出中も揺れる
	void ShakeCamera(float power, int frame) { m_pCamera->Shake(power, frame); }

	//ボスの登場や撃破の演出中か//シーンは演出が終わるまで勝ち負けの切り替えを待つ
	bool IsBossCutscenePlaying()const { return m_bossCutscene.IsPlaying(); }
	//UI用//演出がどこまで進んだか
	const BossCutscene& GetBossCutscene()const { return m_bossCutscene; }

	//戦闘中だけゲートを閉じる
	void UpdateGate(bool shouldClose, const std::vector<SpawnTable::GateInfo>& gates);

	//ステージ→ウィッチタイムの色→プレイヤー、敵、飛び道具→エフェクトの順に描く
	//ここで描いたものだけ歪ませる//UIはシーンがこの後に描くので歪まない
	void Draw();
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
	//ロックオンしているか
	bool IsLockedOn()const;
	//ロックオンしている敵//いなければ空//返したshared_ptrはその場で使うだけにする
	std::shared_ptr<const Character> GetLockOnTarget()const { return m_lockOn.GetTarget(); }
	//敵の一覧//ロックオンやHPバーなど、weak_ptrで敵を覚えておく処理用
	std::vector<std::shared_ptr<Character>> GetEnemies()const;

private:
	//キャラクターの更新から攻撃判定とカメラまで進める
	void UpdateWorld();
	//カメラだけ進める//ヒットストップ中もシェイクは動かす
	//演出用のカメラを使っている間は、そちらのポーズを映す
	void UpdateCamera();
	//ジャスト回避の輪とウィッチタイムの揺らぎを進めてシェーダーに渡す値を決める
	//中心はプレイヤーの画面上の位置なのでカメラを動かした後に呼ぶ
	void UpdateDistortion();
	//死亡演出が終わって消えた敵を消す
	//このフレームの処理が全部終わってから呼ぶ
	void RemoveVanishedEnemies();

	//敵の一覧//その場で使うだけの処理用。フレームをまたいで持たない
	std::vector<Enemy*> GetEnemyList()const;
	//プレイヤーと全部の敵//攻撃の判定など、その場で使うだけの処理用
	std::vector<Character*> GetAllCharacters()const;

	std::shared_ptr<Stage> m_pStage;
	std::shared_ptr<Player> m_pPlayer;
	std::shared_ptr<Camera> m_pCamera;
	//演出用のカメラ
	EventCamera m_eventCamera;
	//ボスの登場と撃破の演出
	BossCutscene m_bossCutscene;

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

	//画面の歪み
	ScreenDistortion m_distortion;
	//ジャスト回避の歪みが始まってから経ったフレーム数//出ていないときは-1
	int m_distortionFrame = -1;
	//ウィッチタイム中の揺らぎの今の強さ
	float m_waveStrength = 0.0f;
	//揺らぎを動かす時間
	float m_waveTime = 0.0f;
	//ウィッチタイムの背景の暗い紫の濃さの割合//0で重ねない、1で一番濃い
	//入るときは波紋が広がるのに合わせて濃くし、終わるときはゆっくり戻す
	float m_witchOverlayRate = 0.0f;
	//波紋が通ったところから色を変えている途中か//ウィッチタイムに入るときのジャスト回避だけtrue
	bool m_isGradeSweeping = false;
};
