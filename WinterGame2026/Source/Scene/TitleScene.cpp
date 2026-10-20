#include "TitleScene.h"
#include "DxLib.h"
#include "SceneController.h"
#include "GameScene.h"
#include "../System/Input.h"
#include "../Common/FontManager.h"
#include "../Game.h"

namespace
{
	//タイトルロゴの文字の大きさ
	constexpr int kTitleFontSize = 88;
	//案内文の文字の大きさ
	constexpr int kGuideFontSize = 32;
	//タイトルロゴの中心の高さ
	constexpr int kTitlePosY = 260;
	//案内文の中心の高さ
	constexpr int kGuidePosY = 500;
	//案内文の点滅の周期(フレーム)
	constexpr int kBlinkInterval = 60;
}

TitleScene::TitleScene(SceneController& controller) :
	Scene(controller)
{
}

void TitleScene::Init()
{
	m_frameCount = 0;
}

void TitleScene::Update()
{
	m_frameCount++;

	if (Input::Instance().IsTriggered("ok"))
	{
		m_controller.ChangeScene(std::make_shared<GameScene>(m_controller));
	}
}

void TitleScene::Draw()
{
	auto& font = FontManager::Instance();
	int centerX = Game::kScreenWidth / 2;

	//タイトルロゴ
	font.DrawCenteredText(centerX, kTitlePosY, "Kudonetta", GetColor(200, 120, 255), kTitleFontSize, GetColor(0, 0, 0));

	//案内文(周期の前半だけ表示して点滅させる)
	if (m_frameCount % kBlinkInterval < kBlinkInterval / 2)
	{
		font.DrawCenteredText(centerX, kGuidePosY, "PRESS ENTER", GetColor(255, 255, 255), kGuideFontSize, GetColor(0, 0, 0));
	}
}
