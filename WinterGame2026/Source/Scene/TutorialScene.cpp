#include "TutorialScene.h"
#include "DxLib.h"
#include "../System/Timer.h"
#include "../Effect/EffectManager.h"
#include "../Common/FontManager.h"
#include "../Game.h"
#include "SceneController.h"
#include "PauseScene.h"
#include "../System/Input.h"
#include "../UI/TutorialTaskUI.h"
#include "../UI/PlayerHPUI.h"
#include <string>
#include <utility>

namespace
{
	//完了と見せてから完了メニューを出すまでのフレーム数
	//最後の課題のボタン連打がそのままメニューの決定にならないように間をあける
	constexpr int kFinishMenuWaitFrame = 90;

	//ステップを飛ばすのに長押しするフレーム数//うっかり押しても飛ばないように少し長めにする
	constexpr int kSkipHoldFrame = 60;

	//画面上のTUTORIALの表示
	constexpr int kLabelFontSize = 32;
	constexpr int kLabelPosY = 40;

	//画面下の案内の表示//課題は右上のTutorialTaskUIが出す
	constexpr int kGuideFontSize = 32;
	constexpr int kGuidePosY = 640;
}

TutorialScene::TutorialScene(SceneController& controller) :
	Scene(controller)
{
}

TutorialScene::~TutorialScene()
{
	//エフェクトとスロー、ヒットストップは次のシーンに持ち越さない
	EffectManager::Instns().StopAll();
	Timer::Instance().Reset();
}

void TutorialScene::Init()
{
	//前のゲームのスローとヒットストップが残らないようにする
	Timer::Instance().Reset();

	//Zバッファの設定
	SetUseZBuffer3D(true);//Zバッファを使う
	SetWriteZBuffer3D(true);//描画する物体はZバッファにも距離を書き込む

	//チュートリアルの部屋、プレイヤー、カメラを用意する//部屋から出られないのでゲートはいらない
	m_game.Init(Stage::Type::Tutorial, 0);

	//最初のステップから始める
	m_guide.Init(0);

	//最初のステップの敵を出す//敵はプレイヤーを狙うのでプレイヤーの後に出す
	UpdateStepEnemies();

	//UIを登録してからまとめて初期化する
	//右上の課題//進行役はこのシーンが持っていてUIより長く残る
	//長押しの進み具合を渡すので、持ち主をm_uiManagerに渡す前に借りておく
	auto taskUI = std::make_unique<TutorialTaskUI>(m_guide);
	m_pTaskUI = taskUI.get();
	m_uiManager.Add(std::move(taskUI));
	//HPバー//プレイヤーはm_game.Initで作られるのでその後に渡す
	m_uiManager.Add(std::make_unique<PlayerHPUI>(m_game.GetPlayer()));
	m_uiManager.Init();
}

void TutorialScene::Update()
{
#ifdef _DEBUG
	m_debug.Update(m_game.GetDebugTargets());
	//一時停止中とスロー中はゲームを進めない
	if (!m_debug.ShouldUpdateGame()) return;
#endif // _DEBUG

	//ポーズはゲームを進める前に見る//積んだメニューが閉じるまでこのシーンは更新されない
	if (Input::Instance().IsTriggered("pause"))
	{
		OpenPauseMenu();
		return;
	}

	//タイマー、キャラクター、攻撃判定、カメラ、エフェクト、消えた敵の片付けまで
	const bool isAdvanced = m_game.Update();

	 std::vector<BattleEvent> events = m_game.TakeBattleEvents();
	 //ヒットストップ中もUIの演出は動かす
	 m_uiManager.Update(Input::Instance());
	 if (!isAdvanced)return;
	 m_frameCount++;
	 //このフレームに起きたことをまとめてチュートリアルの進行役に渡す
	 TutorialInfo info;
	 info.isMoving = m_game.GetPlayer().GetMoveInput().SqMagnitude() > 0.0f;
	 info.isLockedOn = m_game.IsLockedOn();
	 info.events = events;
	 info.isWitchTime = Timer::Instance().IsEnemySlow();
	 info.player = &m_game.GetPlayer();
	 m_guide.Update(info);

	 //長押しで飛ばす//飛ばした先の敵をこのフレームで出せるように敵の出し直しより前
	 UpdateSkip();

	 //次のステップへ進んでいたら敵を出し直す
	 UpdateStepEnemies();

	 UpdateFinishMenu();
}

void TutorialScene::UpdateFinishMenu()
{
	//まだ課題が残っているか、もうメニューを出したあと
	if (!m_guide.IsFinished() || m_isFreePractice) return;

	//完了の文字を少し見せてから出す
	m_finishFrame++;
	if (m_finishFrame < kFinishMenuWaitFrame) return;

	//戻ってきたら自由練習にする//メニューを出すのはこの一度だけ
	m_isFreePractice = true;
	m_controller.PushScene(std::make_shared<PauseScene>(m_controller, "チュートリアル完了",
		std::vector<PauseScene::Item>{
			{ "ステージ1へ", PauseScene::Action::Stage1 },
			{ "練習を続ける", PauseScene::Action::Resume },
			{ "セレクトに戻る", PauseScene::Action::Select },
		}, 0));
}

void TutorialScene::OpenPauseMenu()
{
	std::vector<PauseScene::Item> items;
	items.push_back({ "続ける", PauseScene::Action::Resume });
	//課題を飛ばして本編へ行けないように、全部終わるまでは出さない
	if (m_guide.IsFinished())
	{
		items.push_back({ "ステージ1へ", PauseScene::Action::Stage1 });
	}
	items.push_back({ "セレクトに戻る", PauseScene::Action::Select });

	//続けるを選んでおく//うっかり決定しても戻るだけで済む
	m_controller.PushScene(std::make_shared<PauseScene>(m_controller, "PAUSE", std::move(items), 0));
}

bool TutorialScene::IsSkipHeld() const
{
	const Input& input = Input::Instance();
	//LBはロックオンと同じボタン//RBと両方押しているときだけにして、ロックオンだけで飛ばないようにする
	const bool isPadHeld = input.IsPressed("target") && input.IsPressed("skipRB");
	return isPadHeld || input.IsPressed("skipKey");
}

void TutorialScene::UpdateSkip()
{
	const bool isHeld = IsSkipHeld();

	//飛ばした後は一度離すまで待つ
	if (m_isWaitSkipRelease)
	{
		if (!isHeld) m_isWaitSkipRelease = false;
		m_skipHoldFrame = 0;
	}
	//全部終わったら飛ばすステップがない//離したら最初から数え直す
	else if (!isHeld || m_guide.IsFinished())
	{
		m_skipHoldFrame = 0;
	}
	else
	{
		m_skipHoldFrame++;
		if (m_skipHoldFrame >= kSkipHoldFrame)
		{
			m_guide.SkipStep();
			m_skipHoldFrame = 0;
			m_isWaitSkipRelease = true;
		}
	}

	//ゲージに出す進み具合
	m_pTaskUI->SetSkipRate(static_cast<float>(m_skipHoldFrame) / static_cast<float>(kSkipHoldFrame));
}

void TutorialScene::UpdateStepEnemies()
{
	//全部終わったら敵はそのまま残しておく
	const TutorialStep* step = m_guide.GetCurrentStep();
	if (step == nullptr) return;
	//同じステップの敵はもう出してある
	if (step == m_spawnedStep) return;

	//前のステップの敵を消してから、このステップの敵を出す
	m_game.ClearEnemies();
	for (const TutorialEnemy& enemy : step->enemies)
	{
		m_game.SpawnEnemy(enemy.name, enemy.groundPos);
	}
	m_spawnedStep = step;
	m_game.GetPlayer().SetUndying(!step->canDie);
}

void TutorialScene::Draw()
{
	//ステージウィッチタイムの色キャラクターエフェクト//歪みもかけて出す
	m_game.Draw();

	//右上の課題とHPバー//歪みの後に描くので歪まない
	m_uiManager.Draw();

	auto& font = FontManager::Instance();
	const int centerX = Game::kScreenWidth / 2;
	const unsigned int edgeColor = GetColor(0, 0, 0);

	//本編と見分けがつくように出しておく
	font.DrawCenteredText(centerX, kLabelPosY, "TUTORIAL", GetColor(200, 120, 255), kLabelFontSize, edgeColor);

	//全部終わったあとの案内//課題があるあいだは右上のTutorialTaskUIが出す
	if (m_isFreePractice)
	{
		//ポーズからメニューを開けることを伝える
		font.DrawCenteredText(centerX, kGuidePosY, "自由に練習しよう　ポーズでメニュー", GetColor(255, 255, 255), kGuideFontSize, edgeColor);
	}
	else if (m_guide.IsFinished())
	{
		font.DrawCenteredText(centerX, kGuidePosY, "チュートリアル完了", GetColor(255, 255, 255), kGuideFontSize, edgeColor);
	}

#ifdef _DEBUG
	//当たり判定、ロックオン範囲、敵の範囲など
	m_game.DrawDebug(m_debug);
	//文字情報
	m_debug.Draw(m_game.GetDebugTargets(), m_game.GetCamera(), m_frameCount);
#endif //_DEBUG
}
