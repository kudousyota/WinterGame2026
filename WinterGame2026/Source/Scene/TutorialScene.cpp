#include "TutorialScene.h"
#include "DxLib.h"
#include "../System/Timer.h"
#include "../Effect/EffectManager.h"
#include "../Common/FontManager.h"
#include "../Game.h"
#include <string>

namespace
{
	//置いておくダミー
	//TutorialGuideができたらステップごとに出し直す形にしてそちらへ移す
	struct DummyPlacement
	{
		//SpawnEnemyに渡す名前
		const char* name;
		//地面の位置
		Vector3 groundPos;
	};
	//吹っ飛ばされて消えたダミーは、ここに書いた場所に戻ってくる
	//攻撃してくるダミーは攻撃範囲に入らないと攻撃しないので間を空けておく
	const DummyPlacement kDummies[] =
	{
		//棒立ち//攻撃の練習用
		{ "Dummy", Vector3(0.0f, 0.0f, 600.0f) },
		//攻撃してくる//回避とジャスト回避の練習用
		{ "DummyAttack", Vector3(-500.0f, 0.0f, 1000.0f) },
		//浮いている//空中攻撃・叩き落としの練習用
		{ "DummyFloat", Vector3(500.0f, 0.0f, 1000.0f) },
	};

	//画面上のTUTORIALの表示
	constexpr int kLabelFontSize = 32;
	constexpr int kLabelPosY = 40;

	//画面下の課題の表示
	constexpr int kGuideFontSize = 32;
	constexpr int kGuidePosY = 640;
}

TutorialScene::TutorialScene(SceneController& controller) :
	Scene(controller)
{
}

TutorialScene::~TutorialScene()
{
	//エフェクトとスローヒットストップは次のシーンに持ち越さない
	EffectManager::Instns().StopAll();
	Timer::Instance().Reset();
}

void TutorialScene::Init()
{
	//前のゲームのスローヒットストップが残らないようにする
	Timer::Instance().Reset();

	//Zバッファの設定
	SetUseZBuffer3D(true);//Zバッファを使います
	SetWriteZBuffer3D(true);//描画する物体はZバッファにも距離を書き込む

	//チュートリアルの部屋、プレイヤー、カメラを用意する//部屋から出られないのでゲートはいらない
	m_game.Init(Stage::Type::Tutorial, 0);

	//敵はプレイヤーを狙うのでプレイヤーの後に出す
	for (const DummyPlacement& dummy : kDummies)
	{
		m_game.SpawnEnemy(dummy.name, dummy.groundPos);
	}

	//最初のステップから始める
	m_guide.Init(0);
}

void TutorialScene::Update()
{
#ifdef _DEBUG
	m_debug.Update(m_game.GetDebugTargets());
	//一時停止スロー中はゲームを進めない
	if (!m_debug.ShouldUpdateGame()) return;
#endif // _DEBUG

	//タイマーキャラクター攻撃判定カメラエフェクト消えた敵の片付けまで
	const bool isAdvanced = m_game.Update();

	//戦闘のイベントはまだ使わないが溜まり続けないように受け取っておく
	//パンチやキックのステップを足したら、TutorialInfoに入れて達成の判定に使う
	m_game.TakeBattleEvents();

	//ヒットストップ中は止まっているので数えない
	if (!isAdvanced) return;
	m_frameCount++;

	//このフレームに起きたことをまとめてチュートリアルの進行役に渡す
	TutorialInfo info;
	info.isMoving = m_game.GetPlayer().GetMoveInput().SqMagnitude() > 0.0f;
	info.isLockedOn = m_game.IsLockedOn();
	m_guide.Update(info);
}

void TutorialScene::Draw()
{
	//ステージウィッチタイムの色キャラクターエフェクト
	m_game.Draw();

	auto& font = FontManager::Instance();
	const int centerX = Game::kScreenWidth / 2;
	const unsigned int edgeColor = GetColor(0, 0, 0);

	//本編と見分けがつくように出しておく
	font.DrawCenteredText(centerX, kLabelPosY, "TUTORIAL", GetColor(200, 120, 255), kLabelFontSize, edgeColor);

	//画面下の課題
	if (m_guide.IsFinished())
	{
		font.DrawCenteredText(centerX, kGuidePosY, "チュートリアル完了", GetColor(255, 255, 255), kGuideFontSize, edgeColor);
	}
	else if (m_guide.IsCleared())
	{
		//達成したことを少しのあいだ見せる
		font.DrawCenteredText(centerX, kGuidePosY, "OK!", GetColor(255, 220, 80), kGuideFontSize, edgeColor);
	}
	else
	{
		//課題と何回できたか
		const TutorialStep* step = m_guide.GetCurrentStep();
		std::string text = step->text;
		text += "  " + std::to_string(m_guide.GetCount()) + " / " + std::to_string(step->requiredCount);
		font.DrawCenteredText(centerX, kGuidePosY, text, GetColor(255, 255, 255), kGuideFontSize, edgeColor);
	}

#ifdef _DEBUG
	//当たり判定、ロックオン範囲、敵の範囲など
	m_game.DrawDebug(m_debug);
	//文字情報
	m_debug.Draw(m_game.GetDebugTargets(), m_game.GetCamera(), m_frameCount);
#endif //_DEBUG
}
