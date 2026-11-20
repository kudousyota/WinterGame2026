#include "SelectScene.h"
#include "DxLib.h"
#include <cmath>
#include "SceneController.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "TutorialScene.h"
#include "../System/Input.h"
#include "../Common/FontManager.h"
#include "../Game.h"
#include "PauseScene.h"

namespace
{
	//選べる項目はSelectScene.hに置いている
	using Item = SelectScene::Item;
	constexpr int kItemNum = static_cast<int>(Item::Num);

	//項目ごとの表示内容
	struct ItemInfo
	{
		//表示名
		const char* label;
		//選んでいるときに画面下に出す説明
		const char* description;
		//まだ選べないか
		bool isLocked;
	};

	//Itemの順番どおりに並べる
	const ItemInfo kItems[kItemNum] =
	{
		{ "チュートリアル", "操作と戦い方を覚えよう", false },
		{ "ステージ1", "神殿の敵を全滅させよう", false },
		{ "ステージ2", "Coming Soon", true },
	};

	//見出し
	constexpr int kHeaderFontSize = 56;
	constexpr int kHeaderPosY = 150;
	//項目
	constexpr int kItemFontSize = 40;
	constexpr int kItemStartY = 300;
	constexpr int kItemSpacing = 80;
	//選んでいる項目の拡大率
	constexpr float kSelectedScale = 1.2f;
	//説明文
	constexpr int kDescFontSize = 24;
	constexpr int kDescPosY = 620;

	//カーソルの三角形
	constexpr int kCursorOffsetX = 200;
	constexpr int kCursorSize = 14;

	//ロック中を決定したときの揺れ
	constexpr int kShakeFrame = 20;
	constexpr float kShakeAmplitude = 8.0f;
	constexpr float kShakeSpeed = 1.5f;
}

SelectScene::SelectScene(SceneController& controller) :
	Scene(controller)
{
}

void SelectScene::Init()
{
	//ステージ1から始める
	m_cursor = static_cast<int>(Item::Stage1);
	m_shakeFrame = 0;
}

void SelectScene::Update()
{
	if (m_shakeFrame > 0) m_shakeFrame--;

	auto& input = Input::Instance();

	//決定を先に見る//画面で見ていた項目が選ばれるようにする
	if (input.IsTriggered("ok"))
	{
		Decide();
		return;
	}
	if (input.IsTriggered("cancel"))
	{
		m_controller.ChangeScene(std::make_shared<TitleScene>(m_controller));
		return;
	}

	MoveCursor(GetMoveDir());
}



int SelectScene::GetMoveDir()const
{
	auto& input = Input::Instance();
	//上下を同時に押したら打ち消しあって動かない
	int dir = 0;
	if (input.IsTriggered("down")) dir++;
	if (input.IsTriggered("up")) dir--;
	return dir;
}

void SelectScene::MoveCursor(int dir)
{
	if (dir == 0) return;
	//別の項目に移ったら揺れは止める
	m_shakeFrame = 0;
	//-1%3が-1の項目の数を足してからあまりを取る
	m_cursor = (m_cursor + dir + kItemNum) % kItemNum;
}

void SelectScene::Decide()
{
	const ItemInfo& info = kItems[m_cursor];
	//選べないことを揺れで伝える
	if (info.isLocked)
	{
		m_shakeFrame = kShakeFrame;
		return;
	}

	switch (static_cast<Item>(m_cursor))
	{
	case Item::Tutorial:
		m_controller.ChangeScene(std::make_shared<TutorialScene>(m_controller));
		break;
	case Item::Stage1:
		m_controller.ChangeScene(std::make_shared<GameScene>(m_controller));
		break;
	case Item::Stage2:
		break;
	default:
		break;
	}
}

void SelectScene::Draw()
{
	auto& font = FontManager::Instance();
	const int centerX = Game::kScreenWidth / 2;

	//色
	const unsigned int kSelectedColor = GetColor(200, 120, 255);
	const unsigned int kNormalColor = GetColor(255, 255, 255);
	const unsigned int kLockedColor = GetColor(120, 120, 120);
	const unsigned int kEdgeColor = GetColor(0, 0, 0);

	//見出し
	font.DrawCenteredText(centerX, kHeaderPosY, "SELECT", kSelectedColor, kHeaderFontSize, kEdgeColor);

	//項目
	for (int i = 0; i < kItemNum; i++)
	{
		const ItemInfo& info = kItems[i];
		const bool isSelected = (i == m_cursor);
		const int y = kItemStartY + i * kItemSpacing;

		//ロック中は選んでいても灰色のままにして、選べる項目と見分けられるようにする
		unsigned int color = kNormalColor;
		if (info.isLocked) color = kLockedColor;
		else if (isSelected) color = kSelectedColor;

		if (!isSelected)
		{
			font.DrawCenteredText(centerX, y, info.label, color, kItemFontSize, kEdgeColor);
			continue;
		}

		//ロック中を決定したら少しのあいだ横に揺らす
		int x = centerX;
		if (m_shakeFrame > 0)
		{
			//残りが減るほど揺れを小さくする
			const float rate = static_cast<float>(m_shakeFrame) / kShakeFrame;
			x += static_cast<int>(std::sin(m_shakeFrame * kShakeSpeed) * kShakeAmplitude * rate);
		}

		//選んでいる項目は大きくして左に三角を付ける
		font.DrawCenteredExtendText(x, y, info.label, color, kItemFontSize, kEdgeColor, kSelectedScale);
		const int cursorX = x - kCursorOffsetX;
		DrawTriangle(cursorX - kCursorSize, y - kCursorSize,
			cursorX + kCursorSize, y,
			cursorX - kCursorSize, y + kCursorSize,
			color, true);
	}

	//選んでいる項目の説明
	font.DrawCenteredText(centerX, kDescPosY, kItems[m_cursor].description, kNormalColor, kDescFontSize, kEdgeColor);
}
