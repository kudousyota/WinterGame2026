#include "MenuCursorUI.h"
#include "DxLib.h"
#include <cmath>

namespace
{
	//矢印にするニードルの画像//ウィッチタイムの演出と同じもの
	const char* const kImagePath = "Data/Image/ui_niidle.png";
	//画像の大きさ//元の画像は256x512
	constexpr double kScale = 0.2;
	//縦向きの画像を倒して先が右を向くようにする
	constexpr double kAngle = DX_PI / 2;
	//文字の中心から矢印の中心までの距離
	constexpr int kOffsetX = 230;
	//左右に揺れる幅と速さ
	constexpr float kSwayAmplitude = 6.0f;
	constexpr float kSwaySpeed = 0.1f;
}

MenuCursorUI::~MenuCursorUI()
{
	//メニューは開くたびに作り直すので、消えるときに画像を手放す
	DeleteGraph(m_handle);
}

void MenuCursorUI::Init()
{
	m_handle = LoadGraph(kImagePath);
	m_frame = 0;
	m_isVisible = true;
}

void MenuCursorUI::Update(Input& input)
{
	m_frame++;
}

void MenuCursorUI::Draw()
{
	if (!m_isVisible) return;

	//項目を指していることが分かるように、先を項目の方へ揺らす
	const int sway = static_cast<int>(std::sin(m_frame * kSwaySpeed) * kSwayAmplitude);

	//細い線がガタガタにならないように、縮めるときはなめらかに描く
	SetDrawMode(DX_DRAWMODE_BILINEAR);
	DrawRotaGraph(m_x - kOffsetX + sway, m_y, kScale, kAngle, m_handle, true);
	SetDrawMode(DX_DRAWMODE_NEAREST);
}

void MenuCursorUI::SetItemPos(int x, int y)
{
	m_x = x;
	m_y = y;
}
