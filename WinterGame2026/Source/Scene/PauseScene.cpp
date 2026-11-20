#include "PauseScene.h"
#include "DxLib.h"
#include <utility>
#include "SceneController.h"
#include "SelectScene.h"
#include "GameScene.h"
#include "../System/Input.h"
#include "../Common/FontManager.h"
#include "../Game.h"

namespace
{
	//後ろの画面を暗くする濃さ//0で透明255で真っ黒
	constexpr int kBackAlpha = 150;

	//見出し
	constexpr int kTitleFontSize = 56;
	constexpr int kTitlePosY = 200;
	//項目
	constexpr int kItemFontSize = 40;
	constexpr int kItemStartY = 330;
	constexpr int kItemSpacing = 80;
	//選んでいる項目の拡大率
	constexpr float kSelectedScale = 1.2f;
}

PauseScene::PauseScene(SceneController& controller, const char* title, std::vector<Item> items, int firstCursor):
	Scene(controller),
	m_title(title),
	m_items(std::move(items)),
	m_cursor(firstCursor)
{
}

void PauseScene::Init()
{
	//カーソルの位置はコンストラクタで受け取ったままにする
	m_cursorUI.Init();
}

void PauseScene::Update()
{
	auto& input = Input::Instance();
	m_cursorUI.Update(input);

	//決定を先に見る//画面で見ていた項目が選ばれるようにする
	if (input.IsTriggered("ok"))
	{
		Decide();
		return;
	}
	//戻るボタンとポーズボタンは閉じてゲームに戻る
	if (input.IsTriggered("cancel") || input.IsTriggered("pause"))
	{
		m_controller.PopScene();
		return;
	}

	MoveCursor(GetMoveDir());
}

int PauseScene::GetMoveDir()const
{
	auto& input = Input::Instance();
	//上下を同時に押したら打ち消しあって動かない
	int dir = 0;
	if (input.IsTriggered("down")) dir++;
	if (input.IsTriggered("up")) dir--;
	return dir;
}

void PauseScene::MoveCursor(int dir)
{
	if (dir == 0) return;
	const int itemNum = static_cast<int>(m_items.size());
	//-1のあまりが-1になるので、項目の数を足してからあまりを取る
	m_cursor = (m_cursor + dir + itemNum) % itemNum;
}

void PauseScene::Decide()
{
	switch (m_items[m_cursor].action)
	{
	case Action::Resume:
		//閉じて下のシーンに戻る
		m_controller.PopScene();
		break;
	case Action::Stage1:
		//ChangeSceneだと下のシーンが残るので、積まれているシーンごと入れ替える
		m_controller.ResetScene(std::make_shared<GameScene>(m_controller));
		break;
	case Action::Select:
		m_controller.ResetScene(std::make_shared<SelectScene>(m_controller));
		break;
	default:
		break;
	}
}

void PauseScene::Draw()
{
	auto& font = FontManager::Instance();
	const int centerX = Game::kScreenWidth / 2;

	//GetColorはDxLibの初期化後に呼ぶ必要があるので、ここで作る
	const unsigned int kSelectedColor = GetColor(200, 120, 255);
	const unsigned int kNormalColor = GetColor(255, 255, 255);
	const unsigned int kEdgeColor = GetColor(0, 0, 0);

	//後ろのゲーム画面を暗くして文字を読みやすくする
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBackAlpha);
	DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, kEdgeColor, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	//見出し
	font.DrawCenteredText(centerX, kTitlePosY, m_title, kSelectedColor, kTitleFontSize, kEdgeColor);

	//項目
	for (int i = 0; i < static_cast<int>(m_items.size()); i++)
	{
		const int y = kItemStartY + i * kItemSpacing;

		if (i != m_cursor)
		{
			font.DrawCenteredText(centerX, y, m_items[i].name, kNormalColor, kItemFontSize, kEdgeColor);
			continue;
		}

		//選んでいる項目は大きくして左に矢印を付ける//色だけに頼らない
		font.DrawCenteredExtendText(centerX, y, m_items[i].name, kSelectedColor, kItemFontSize, kEdgeColor, kSelectedScale);
		m_cursorUI.SetItemPos(centerX, y);
		m_cursorUI.Draw();
	}
}
