#include "TitleScene.h"
#include "DxLib.h"
#include "SceneController.h"
#include "SelectScene.h"
#include "../System/Input.h"
#include "../Common/FontManager.h"
#include "../Game.h"
#include "../Color.h"

namespace
{
	//タイトルロゴの画像//元の画像は1800x580で、まわりは透明
	//ARISSETTA版に戻すときは"Data/Image/title_logo.png"(1500x489)にして、大きさを0.75にする
	const char* const kLogoImagePath = "Data/Image/title_logo_kudonetta2.png";
	//タイトルロゴの大きさ
	constexpr double kLogoScale = 0.65;
	//案内文の文字の大きさ
	constexpr int kGuideFontSize = 32;
	//タイトルロゴの中心の高さ
	constexpr int kTitlePosY = 260;
	//案内文の中心の高さ
	constexpr int kGuidePosY = 500;
	//案内文の点滅の周期(フレーム)
	constexpr int kBlinkInterval = 60;

	//背景のカメラ//画角と映る距離は本編のCameraと同じにする
	constexpr float kFov = DX_PI_F / 3.0f;
	constexpr float kNear = 20.0f;
	constexpr float kFar = 20000.0f;
	//カメラの位置と見る点//神殿前の広場から神殿の正面を見る(1単位1cm)
	const VECTOR kCameraPos = VGet(0.0f, 300.0f, 5000.0f);
	const VECTOR kCameraTarget = VGet(0.0f, 600.0f, 9100.0f);
}

TitleScene::TitleScene(SceneController& controller) :
	Scene(controller)
{
}

TitleScene::~TitleScene()
{
	DeleteGraph(m_logoHandle);
}

void TitleScene::Init()
{
	m_frameCount = 0;
	m_logoHandle = LoadGraph(kLogoImagePath);

	//タイトルでは当たり判定を使わない
	m_stage.Init(Stage::Type::Temple, false);

	//本編ではCamera::Initがやっていることをここでする
	SetupCamera_Perspective(kFov);
	SetCameraNearFar(kNear, kFar);
	SetCameraPositionAndTarget_UpVecY(kCameraPos, kCameraTarget);
	//ライトはカメラの向きから当てる//当てないと神殿の正面が暗くなる
	SetLightDirection(VNorm(VSub(kCameraTarget, kCameraPos)));
}

void TitleScene::Update()
{
	m_frameCount++;

	if (Input::Instance().IsTriggered("ok"))
	{
		m_controller.ChangeScene(std::make_shared<SelectScene>(m_controller));
	}
}

void TitleScene::Draw()
{
	auto& font = FontManager::Instance();
	int centerX = Game::kScreenWidth / 2;

	//ステージは一番奥なので最初に描く
	m_stage.Draw();

	//タイトルロゴ
	//縮めて描くので、ガタガタにならないようになめらかに描く
	SetDrawMode(DX_DRAWMODE_BILINEAR);
	DrawRotaGraph(centerX, kTitlePosY, kLogoScale, 0.0, m_logoHandle, true);
	SetDrawMode(DX_DRAWMODE_NEAREST);

	//案内文(周期の前半だけ表示して点滅させる)
	if (m_frameCount % kBlinkInterval < kBlinkInterval / 2)
	{
		font.DrawCenteredText(centerX, kGuidePosY, "PRESS ENTER", Color::kWhite, kGuideFontSize, Color::kBlack);
	}
}
