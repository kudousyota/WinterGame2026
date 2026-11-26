#include "GameScene.h"
#include "DxLib.h"
#include "../System/Timer.h"
#include "../Game.h"
#include "../Effect/EffectManager.h"
#include "SceneController.h"
#include "GameClearScene.h"
#include "GameOverScene.h"
#include "PauseScene.h"
#include "../System/Input.h"
#include <algorithm>
#include <vector>
#include "../DataLoader/DataManager.h"
#include "../UI/WitchTimeNeedleUI.h"
#include "../UI/PlayerHPUI.h"
#include "../UI/EnemyHPUI.h"
#include "../UI/BossHPUI.h"
#include "../UI/BossCutsceneUI.h"

namespace
{
	//勝ち負けが決まってから画面を切り替えるまでのフレーム数
	constexpr int kGameEndWaitFrame = 120;
}

GameScene::GameScene(SceneController& controller):
Scene(controller),
m_frameCount(0)
{
}

GameScene::~GameScene()
{
	//エフェクトとスロー、ヒットストップは次のシーンに持ち越さない
	EffectManager::Instns().StopAll();
	Timer::Instance().Reset();
}

void GameScene::Init()
{
	//前のスローとヒットストップが残らないようにする
	Timer::Instance().Reset();

	//Zバッファの設定
	SetUseZBuffer3D(true);//Zバッファを使う
	SetWriteZBuffer3D(true);//描画する物体はZバッファにも距離を書き込む

	//ステージ、プレイヤー、カメラ、ゲートを用意する
	const auto& spawnTable = DataManager::GetInstance().GetSpawnTable();
	m_game.Init(Stage::Type::Temple, spawnTable.GetMaxGateNum());

	//敵はプレイヤーを狙うからプレイヤーの後に準備する
	m_spawner.Init(spawnTable);

	//UIを登録してからまとめて初期化する
	m_uiManager.Add(std::make_unique<WitchTimeNeedleUI>());
	//プレイヤーはm_game.Initで作られるのでその後に渡す
	m_uiManager.Add(std::make_unique<PlayerHPUI>(m_game.GetPlayer()));
	//敵のHPバー//敵は後から出てくるのでUIの中で敵の一覧を毎フレーム見る
	m_uiManager.Add(std::make_unique<EnemyHPUI>(m_game));
	//ボスのHPバー//画面の下の真ん中//ボスはBossCutsceneに聞く
	m_uiManager.Add(std::make_unique<BossHPUI>(m_game));
	//ボスの演出の黒帯や文字//ほかのUIの上に重ねるので最後に足す
	m_uiManager.Add(std::make_unique<BossCutsceneUI>(m_game));
	m_uiManager.Init();
}

void GameScene::Update()
{
#ifdef _DEBUG
	m_debug.Update(m_game.GetDebugTargets());
	//一時停止中とスロー中はゲームを進めない
	if (!m_debug.ShouldUpdateGame()) return;
#endif // _DEBUG

	//ポーズはゲームを進める前に見る//積んだメニューが閉じるまでこのシーンは更新されない
	//勝ち負けが決まった後は切り替えを待つだけなので開かない
	if (m_gameEndFrame == 0 && Input::Instance().IsTriggered("pause"))
	{
		OpenPauseMenu();
		return;
	}

	//タイマー、キャラクター、攻撃判定、カメラ、エフェクト、消えた敵の片付けまで
	const bool isAdvanced = m_game.Update();

	//このフレームの戦闘で起きたことを受け取る//受け取らないとGameManagerにたまり続ける
	//ヒットストップ中はキャラクターが止まっていてイベントは出ないので毎フレーム受け取る
	HandleBattleEvents();
	//ウィッチタイムの状態はゲームを進めた後に決まるのでその後に見る
	//ヒットストップ中もUIの演出は動かす
	m_uiManager.Update(Input::Instance());

	//ヒットストップ中は進行も止める
	if (!isAdvanced) return;

	m_frameCount++;

	//消えた敵を外した後だから残っている敵の数で次のウェーブを決められる
	UpdateSpawn();
	//勝ち負けの判定
	CheckGameEnd();
}

void GameScene::Draw()
{
	//ステージ、ウィッチタイムの色、キャラクター、エフェクト//歪みもかけて出す
	m_game.Draw();

	//UIは歪ませたくないから歪みの後に一番手前に描く
	m_uiManager.Draw();
#ifdef _DEBUG
	//どれを出すかはDebugToolのメニューやFキーで切り替える
	if (m_debug.IsShow(DebugTool::Show::Grid)) DrawGrid();
	//当たり判定、ロックオン範囲、敵の範囲など
	m_game.DrawDebug(m_debug);
	//文字情報
	m_debug.Draw(m_game.GetDebugTargets(), m_game.GetCamera(), m_frameCount);
#endif //_DEBUG

}

void GameScene::HandleBattleEvents()
{
	//カメラの揺れとジャスト回避の歪みはGameManagerが出しているので、今ここで行う演出はない
	for (const BattleEvent& event : m_game.TakeBattleEvents())
	{
		switch (event.type)
		{
		case BattleEvent::Type::JustDodge:
			break;
		case BattleEvent::Type::Hit:
			break;
		case BattleEvent::Type::Dodge:
			//今は見た目の演出はない//チュートリアルで回数を数えるのに使う
			break;
		}
	}
}

void GameScene::UpdateSpawn()
{
	//プレイヤー基準の敵はカメラの正面//画面の奥に出す
	const Camera& camera = m_game.GetCamera();
	SpawnContext spawnContext{ m_game.GetPlayer().GetPos(), camera.GetForward(), camera.GetRight() };
	m_spawner.Update(m_game.GetEnemyCount(), spawnContext);
	//戦闘はSpawnerのUpdateの中で始まって終わるのでその後にゲートを合わせる
	m_game.UpdateGate(m_spawner.IsBattleLocked(), m_spawner.GetBattleArea().gates);
	for (const auto& request : m_spawner.TakeSpawnRequests())
	{
		m_game.SpawnEnemy(request.characterName, request.groundPos);
	}
}

void GameScene::CheckGameEnd()
{
	bool isGameOver = m_game.GetPlayer().IsDead();
	//最後のウェーブまで出し終わっていなければクリアではない
	//敵は倒れて消えるまで見せてからクリアにする
	bool isClear = m_spawner.IsAllWavesFinished() && m_game.AreAllEnemiesVanished();
	//まだ勝ち負けが決まっていない
	if (!isGameOver && !isClear) return;
	//ボスの撃破の演出を最後まで見せてから切り替える
	if (m_game.IsBossCutscenePlaying()) return;

	//倒れる様子を少し見せてから切り替える
	m_gameEndFrame++;
	if (m_gameEndFrame < kGameEndWaitFrame) return;

	//相打ちのときはゲームオーバーにする
	if (isGameOver)
	{
		m_controller.ChangeScene(std::make_shared<GameOverScene>(m_controller));
	}
	else
	{
		m_controller.ChangeScene(std::make_shared<GameClearScene>(m_controller));
	}
}

void GameScene::OpenPauseMenu()
{
	//やり直すはステージ1を最初から始めるのと同じなのでStage1を使う
	//続けるを選んでおく//うっかり決定しても戻るだけで済む
	m_controller.PushScene(std::make_shared<PauseScene>(m_controller, "PAUSE",
		std::vector<PauseScene::Item>{
			{ "続ける", PauseScene::Action::Resume },
			{ "やり直す", PauseScene::Action::Stage1 },
			{ "セレクトに戻る", PauseScene::Action::Select },
		}, 0));
}

void GameScene::DrawGrid()
{
	//直線の始点と終点
	VECTOR startPos;
	VECTOR endPos;

	for (int z = -300; z <= 300; z += 100)
	{
		startPos = VGet(-300.0f, 0.0f, static_cast<float>(z));
		endPos = VGet(300.0f, 0.0f, static_cast<float>(z));
		DrawLine3D(startPos, endPos, 0xff0000);
	}
	for (int x = -300; x <= 300; x += 100)
	{
		startPos = VGet(static_cast<float>(x), 0.0f, -300.0f);
		endPos = VGet(static_cast<float>(x), 0.0f, 300.0f);
		DrawLine3D(startPos, endPos, 0x0000ff);
	}
}
