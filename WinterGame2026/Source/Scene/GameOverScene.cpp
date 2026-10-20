#include "GameOverScene.h"
#include "DxLib.h"
#include "SceneController.h"
#include "TitleScene.h"
#include "../System/Input.h"
#include "../Common/FontManager.h"
#include "../Game.h"

namespace
{
	//見出しの文字の大きさ
	constexpr int kTitleFontSize = 80;
	//案内文の文字の大きさ
	constexpr int kGuideFontSize = 32;
	//見出しの中心の高さ
	constexpr int kTitlePosY = 280;
	//案内文の中心の高さ
	constexpr int kGuidePosY = 500;
	//案内文の点滅の周期(フレーム)
	constexpr int kBlinkInterval = 60;
}

GameOverScene::GameOverScene(SceneController& controller) :
	Scene(controller)
{
}

void GameOverScene::Init()
{
	m_frameCount = 0;
}

void GameOverScene::Update()
{
	m_frameCount++;

	if (Input::Instance().IsTriggered("ok"))
	{
		m_controller.ChangeScene(std::make_shared<TitleScene>(m_controller));
	}
}

void GameOverScene::Draw()
{
	auto& font = FontManager::Instance();
	int centerX = Game::kScreenWidth / 2;

	//見出し
	font.DrawCenteredText(centerX, kTitlePosY, "GAME OVER", GetColor(220, 40, 40), kTitleFontSize, GetColor(0, 0, 0));

	//案内文(周期の前半だけ表示して点滅させる)
	if (m_frameCount % kBlinkInterval < kBlinkInterval / 2)
	{
		font.DrawCenteredText(centerX, kGuidePosY, "PRESS ENTER TO TITLE", GetColor(255, 255, 255), kGuideFontSize, GetColor(0, 0, 0));
	}
}
