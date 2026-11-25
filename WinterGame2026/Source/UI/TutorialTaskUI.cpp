#include "TutorialTaskUI.h"
#include "DxLib.h"
#include <string>
#include "../Game/TutorialGuide.h"
#include "../Common/FontManager.h"
#include "../Game.h"
#include "../Math/MathUtil.h"
#include "../System/Timer.h"
#include <algorithm>

namespace
{
	//文字の大きさ//FontManagerで作ってある8刻みのサイズから選ぶ
	constexpr int kStepFontSize = 24;
	constexpr int kTaskFontSize = 32;

	//段の縦の中心
	constexpr int kStepPosY = 70;
	constexpr int kTaskPosY = 118;

	//文字の右端から画面の右端までの間//帯はこの分も右へ伸びて画面の端につく
	constexpr int kRightMargin = 40;
	//帯の左端から文字までの間
	constexpr int kTextPaddingX = 28;
	//文字の上下に足す帯の高さ
	constexpr int kBandPaddingY = 12;
	//帯の下の辺を上の辺より左へずらす量//平行四辺形にして動きを出す
	constexpr int kBandSlant = 14;
	//帯の濃さ//0で透明255で真っ黒
	constexpr int kBandAlpha = 170;

	//課題の文と回数の間
	constexpr int kCountSpacing = 20;

	//ステップが変わってから真ん中で止まって見せるフレーム数
	constexpr int kHoldFrame = 60;
	//真ん中から右上へ動くのにかけるフレーム数
	constexpr int kMoveFrame = 30;
	//真ん中に出すときの文字の縦の中心//少し上にしてキャラクターに重ならないようにする
	constexpr float kCenterPosY = 300.0f;
	//真ん中に出すときの拡大率//右上に着いたら1倍にする
	constexpr float kCenterScale = 1.6f;
	//真ん中に出すときに文字の後ろへ敷く横いっぱいの帯の高さと濃さ
	constexpr int kCenterBandHeight = 96;
	constexpr int kCenterBandAlpha = 150;

	//ステップを飛ばす案内//STEPの段の左に並べる
	const char* const kSkipHintText = "LB+RB長押しでスキップ";
	//案内とSTEPの間
	constexpr int kSkipHintSpacing = 32;
	//長押しのゲージの太さ//STEPの段の帯の下の辺に沿って伸ばす
	constexpr int kSkipGaugeHeight = 4;

	//ウィッチタイム中に出す一言
	const char* const kWitchTimeText = "敵の動きが遅い間に攻撃しよう！";
	constexpr int kWitchTimeFontSize = 40;
	//縦の中心//課題を真ん中に出す位置より上にして重ならないようにする
	constexpr int kWitchTimePosY = 210;
	//出た瞬間の拡大率//ここから1倍へ縮めて目を引く
	constexpr float kWitchTimePopScale = 1.5f;
	//1倍まで縮むのにかけるフレーム数
	constexpr int kWitchTimePopFrame = 12;
	//後ろに敷く横いっぱいの帯の高さと濃さ
	constexpr int kWitchTimeBandHeight = 72;
	constexpr int kWitchTimeBandAlpha = 140;
}

TutorialTaskUI::TutorialTaskUI(const TutorialGuide& guide)
{
	m_pGuide = &guide;
}

void TutorialTaskUI::Init()
{
	m_isVisible = true;
}

void TutorialTaskUI::Update(Input& input)
{
	//ウィッチタイムに入っている間だけ数える//終わったら次に入ったとき最初から出し直す
	if (Timer::Instance().IsEnemySlow())
	{
		m_witchTimeFrame++;
	}
	else
	{
		m_witchTimeFrame = -1;
	}

	//フレームはDrawではなくここで数える//デバッグの一時停止中に動きまで進まないようにする
	const TutorialStep* step = m_pGuide->GetCurrentStep();
	if (step != m_shownStep)
	{
		//次のステップに変わったので真ん中から出し直す
		m_shownStep = step;
		m_frame = 0;
		return;
	}
	m_frame++;
}

float TutorialTaskUI::GetMoveRate() const
{
	//止まっている間はマイナス、着いた後は1を超えるので0～1に収める
	const float t = static_cast<float>(m_frame - kHoldFrame) / static_cast<float>(kMoveFrame);
	return std::clamp(t, 0.0f, 1.0f);
}

void TutorialTaskUI::Draw()
{
	if (!m_isVisible) return;

	//ウィッチタイムの一言は自由練習中も出す//課題より先に見る
	DrawWitchTimeMessage();

	//全部終わったら課題はないので出さない
	const TutorialStep* step = m_pGuide->GetCurrentStep();
	if (step == nullptr) return;

	auto& font = FontManager::Instance();
	//GetColorはDxLibの初期化後に呼ぶ必要があるので、ここで作る
	const unsigned int stepColor = GetColor(200, 120, 255);
	const unsigned int textColor = GetColor(255, 255, 255);
	const unsigned int countColor = GetColor(200, 120, 255);
	const unsigned int clearColor = GetColor(255, 220, 80);
	const unsigned int edgeColor = GetColor(0, 0, 0);
	const unsigned int hintColor = GetColor(170, 170, 170);

	//文字の右端//ここから幅の分だけ左に置く
	const int textRight = Game::kScreenWidth - kRightMargin;

	//上の段：飛ばし方の案内とステップ番号//画面には1から数えて出す
	const std::string stepText = "STEP " + std::to_string(m_pGuide->GetStepIndex() + 1) + " / " + std::to_string(m_pGuide->GetStepNum());
	const int stepLeft = textRight - font.GetTextWidth(stepText, kStepFontSize);
	const int hintLeft = stepLeft - kSkipHintSpacing - font.GetTextWidth(kSkipHintText, kStepFontSize);
	const int stepBandLeft = hintLeft - kTextPaddingX;
	const int stepBandHeight = kStepFontSize + kBandPaddingY * 2;
	DrawBand(stepBandLeft, kStepPosY, stepBandHeight);
	font.DrawLeftText(hintLeft, kStepPosY, kSkipHintText, hintColor, kStepFontSize, edgeColor);
	font.DrawLeftText(stepLeft, kStepPosY, stepText, stepColor, kStepFontSize, edgeColor);

	//長押しのゲージ//押している間だけ、帯の下の辺に沿って左から伸ばす
	if (m_skipRate > 0.0f)
	{
		const int gaugeBottom = kStepPosY - stepBandHeight / 2 + stepBandHeight;
		const int gaugeRight = stepBandLeft + static_cast<int>((Game::kScreenWidth - stepBandLeft) * m_skipRate);
		DrawBox(stepBandLeft, gaugeBottom - kSkipGaugeHeight, gaugeRight, gaugeBottom, stepColor, true);
	}

	//達成したことを少しのあいだ見せる
	if (m_pGuide->IsCleared())
	{
		const std::string clearText = "OK!";
		const int clearLeft = textRight - font.GetTextWidth(clearText, kTaskFontSize);
		DrawBand(clearLeft - kTextPaddingX, kTaskPosY, kTaskFontSize + kBandPaddingY * 2);
		font.DrawLeftText(clearLeft, kTaskPosY, clearText, clearColor, kTaskFontSize, edgeColor);
		return;
	}

	//課題の文と何回できたか//回数は色を変えて文の右に並べる
	const std::string taskText = step->text;
	const std::string countText = std::to_string(m_pGuide->GetCount()) + " / " + std::to_string(step->requiredCount);
	const int countWidth = font.GetTextWidth(countText, kTaskFontSize);
	const int taskWidth = font.GetTextWidth(taskText, kTaskFontSize);
	const int taskLeft = textRight - countWidth - kCountSpacing - taskWidth;

	//まだ右上に着いていなければ課題の文だけを真ん中から動かして描く
	const float moveRate = GetMoveRate();
	if (moveRate < 1.0f)
	{
		//シュッと動いてふわっと止まるように割合を曲げる
		const float eased = MathUtil::EaseOutCubic(moveRate);

		//真ん中の帯は動き出したら薄くしていく
		const int centerBandAlpha = static_cast<int>(kCenterBandAlpha * (1.0f - eased));
		if (centerBandAlpha > 0)
		{
			const int bandTop = static_cast<int>(kCenterPosY) - kCenterBandHeight / 2;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, centerBandAlpha);
			DrawBox(0, bandTop, Game::kScreenWidth, bandTop + kCenterBandHeight, edgeColor, true);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		//着く場所は、止まった表示で文を描く場所の真ん中
		//1倍で同じ場所に着くので、描き方が切り替わっても文字がずれない
		const float endX = static_cast<float>(taskLeft + taskWidth / 2);
		const float endY = static_cast<float>(kTaskPosY);
		const float x = MathUtil::Lerp(Game::kScreenWidth * 0.5f, endX, eased);
		const float y = MathUtil::Lerp(kCenterPosY, endY, eased);
		const float scale = MathUtil::Lerp(kCenterScale, 1.0f, eased);
		font.DrawCenteredExtendText(static_cast<int>(x), static_cast<int>(y), taskText, textColor, kTaskFontSize, edgeColor, scale);
		return;
	}

	DrawBand(taskLeft - kTextPaddingX, kTaskPosY, kTaskFontSize + kBandPaddingY * 2);
	font.DrawLeftText(taskLeft, kTaskPosY, taskText, textColor, kTaskFontSize, edgeColor);
	font.DrawLeftText(textRight - countWidth, kTaskPosY, countText, countColor, kTaskFontSize, edgeColor);
}

void TutorialTaskUI::DrawWitchTimeMessage() const
{
	//ウィッチタイムに入っていない
	if (m_witchTimeFrame < 0) return;

	//出た瞬間に大きく、すぐ1倍へ縮める
	const float popRate = (std::min)(static_cast<float>(m_witchTimeFrame) / static_cast<float>(kWitchTimePopFrame), 1.0f);
	const float scale = MathUtil::Lerp(kWitchTimePopScale, 1.0f, MathUtil::EaseOutCubic(popRate));

	//後ろの紫の画面でも読めるように横いっぱいの帯を敷く
	const int bandTop = kWitchTimePosY - kWitchTimeBandHeight / 2;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, kWitchTimeBandAlpha);
	DrawBox(0, bandTop, Game::kScreenWidth, bandTop + kWitchTimeBandHeight, GetColor(0, 0, 0), true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	FontManager::Instance().DrawCenteredExtendText(Game::kScreenWidth / 2, kWitchTimePosY, kWitchTimeText,
		GetColor(255, 220, 80), kWitchTimeFontSize, GetColor(0, 0, 0), scale);
}

void TutorialTaskUI::DrawBand(int left, int centerY, int height) const
{
	const int top = centerY - height / 2;
	const int bottom = top + height;

	//後ろのゲーム画面が少し透けるようにする
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBandAlpha);
	//左上、右上、右下、左下の順//左下だけ左へずらして斜めにする
	DrawQuadrangle(left, top, Game::kScreenWidth, top, Game::kScreenWidth, bottom, left - kBandSlant, bottom, GetColor(0, 0, 0), true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
