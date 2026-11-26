#include "BossCutsceneUI.h"
#include "DxLib.h"
#include <algorithm>
#include "../Game/GameManager.h"
#include "../Common/FontManager.h"
#include "../Game.h"
#include "../Math/MathUtil.h"
#include "../Color.h"

namespace
{
	using Phase = BossCutscene::Phase;

	//黒帯の高さ(上と下それぞれ)
	constexpr int kLetterboxHeight = 80;
	//黒帯が出入りする速さ(0～1)
	constexpr float kLetterboxSpeed = 0.12f;

	//名前//叫び始めてから少し待って、叫びのピークに合わせて出す
	const char* const kBossName = "WARROK";
	const char* const kBossTitle = "神殿の番獣";
	constexpr int kNameStartFrame = 30;
	constexpr int kNameFontSize = 72;
	constexpr int kTitleFontSize = 24;
	//名前の縦の中心//下の黒帯の少し上
	constexpr int kNamePosY = Game::kScreenHeight - kLetterboxHeight - 70;
	//肩書きは名前の上に小さく出す
	constexpr int kTitleOffsetY = 58;
	//名前の下に引く線の太さと、名前の中心からの距離
	constexpr int kNameLineHeight = 2;
	constexpr int kNameLineOffsetY = 46;
	//線の長さ//出始めは0から伸ばす
	constexpr int kNameLineHalfWidth = 260;
	constexpr int kNameLineGrowFrame = 20;
	//名前が濃くなる、薄くなる速さ(0～1)
	constexpr float kNameFadeSpeed = 0.08f;

	//とどめの白い光が消えるまでのフレーム数//ヒットストップの間に薄くなっていく
	constexpr int kFlashFrame = 24;
	//光り始めの濃さ
	constexpr int kFlashAlpha = 220;

	//「BOSS DEFEATED」
	const char* const kDefeatedText = "BOSS DEFEATED";
	constexpr int kDefeatedFontSize = 80;
	//縦の中心//真ん中より少し上
	constexpr int kDefeatedPosY = 300;
	//出た瞬間の拡大率//ここから1倍へ縮める
	constexpr float kDefeatedPopScale = 1.6f;
	constexpr int kDefeatedPopFrame = 20;
	//後ろに敷く横いっぱいの帯
	constexpr int kDefeatedBandHeight = 120;
	constexpr int kDefeatedBandAlpha = 150;

	//黒帯を出す段階
	bool IsLetterboxPhase(Phase phase)
	{
		switch (phase)
		{
		case Phase::IntroAppear:
		case Phase::IntroRoar:
		case Phase::DefeatSlow:
		case Phase::DefeatFall:
		case Phase::DefeatBanner:
		case Phase::DefeatDone:
			return true;
		default:
			//カメラを戻している間に引っ込めて、戻りきる頃には普段の画面にする
			return false;
		}
	}

	//0～1の割合を0～255の濃さにする
	int ToAlpha(float rate)
	{
		return static_cast<int>(255.0f * std::clamp(rate, 0.0f, 1.0f));
	}
}

BossCutsceneUI::BossCutsceneUI(const GameManager& game)
{
	m_pGame = &game;
}

void BossCutsceneUI::Init()
{
	m_isVisible = true;
}

void BossCutsceneUI::Update(Input& input)
{
	const BossCutscene& cutscene = m_pGame->GetBossCutscene();
	const Phase phase = cutscene.GetPhase();

	//段階が変わった瞬間に始める演出
	if (phase != m_prevPhase)
	{
		if (phase == Phase::DefeatSlow) m_flashFrame = 0;
		if (phase == Phase::DefeatBanner) m_defeatedFrame = 0;
		m_prevPhase = phase;
	}

	//黒帯//ヒットストップ中もこのUpdateは呼ばれるので、止まっている間も動く
	const float letterboxGoal = IsLetterboxPhase(phase) ? 1.0f : 0.0f;
	m_letterboxRate += (letterboxGoal - m_letterboxRate) * kLetterboxSpeed;

	//名前は叫んでいる間だけ出す//飛ばしたときやカメラを戻し始めたら消していく
	const bool isNameShown = (phase == Phase::IntroRoar && cutscene.GetPhaseFrame() >= kNameStartFrame);
	if (isNameShown)
	{
		if (m_nameFrame < 0) m_nameFrame = 0;
		m_nameFrame++;
		m_nameRate = (std::min)(m_nameRate + kNameFadeSpeed, 1.0f);
	}
	else
	{
		m_nameRate = (std::max)(m_nameRate - kNameFadeSpeed, 0.0f);
		//消えきったら次に出すときは線を伸ばし直す
		if (m_nameRate <= 0.0f) m_nameFrame = -1;
	}

	if (m_flashFrame >= 0)
	{
		m_flashFrame++;
		if (m_flashFrame > kFlashFrame) m_flashFrame = -1;
	}
	if (m_defeatedFrame >= 0)
	{
		m_defeatedFrame++;
	}
	//演出していないときは消しておく//ステージをやり直したときに残らないようにする
	if (phase == Phase::None)
	{
		m_defeatedFrame = -1;
	}
}

void BossCutsceneUI::Draw()
{
	if (!m_isVisible) return;

	DrawLetterbox();
	DrawBossName();
	DrawDefeated();
	//白い光は全部の上に重ねる
	DrawFlash();
}

void BossCutsceneUI::DrawLetterbox() const
{
	const int height = static_cast<int>(kLetterboxHeight * MathUtil::EaseOutCubic(m_letterboxRate));
	if (height <= 0) return;

	//上から下へ、下から上へ伸びてくる
	DrawBox(0, 0, Game::kScreenWidth, height, Color::kBlack, true);
	DrawBox(0, Game::kScreenHeight - height, Game::kScreenWidth, Game::kScreenHeight, Color::kBlack, true);
}

void BossCutsceneUI::DrawBossName() const
{
	if (m_nameRate <= 0.0f) return;

	auto& font = FontManager::Instance();
	const int centerX = Game::kScreenWidth / 2;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, ToAlpha(m_nameRate));
	//肩書き//紫で小さく
	font.DrawCenteredText(centerX, kNamePosY - kTitleOffsetY, kBossTitle, Color::kPurple, kTitleFontSize, Color::kBlack);
	//名前//金色で大きく
	font.DrawCenteredText(centerX, kNamePosY, kBossName, Color::kGold, kNameFontSize, Color::kBlack);

	//名前の下の線//真ん中から左右へ伸ばす
	const float growRate = (std::min)(static_cast<float>((std::max)(m_nameFrame, 0)) / kNameLineGrowFrame, 1.0f);
	const int halfWidth = static_cast<int>(kNameLineHalfWidth * MathUtil::EaseOutCubic(growRate));
	if (halfWidth > 0)
	{
		const int lineTop = kNamePosY + kNameLineOffsetY;
		DrawBox(centerX - halfWidth, lineTop, centerX + halfWidth, lineTop + kNameLineHeight, Color::kPurple, true);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void BossCutsceneUI::DrawFlash() const
{
	if (m_flashFrame < 0) return;

	//一瞬真っ白に近くして、だんだん透明にする
	const float rate = 1.0f - static_cast<float>(m_flashFrame) / kFlashFrame;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(kFlashAlpha * rate));
	DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, Color::kWhite, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void BossCutsceneUI::DrawDefeated() const
{
	if (m_defeatedFrame < 0) return;

	//出た瞬間に大きく、すぐ1倍へ縮めながら濃くする
	const float popRate = (std::min)(static_cast<float>(m_defeatedFrame) / kDefeatedPopFrame, 1.0f);
	const float eased = MathUtil::EaseOutCubic(popRate);
	const float scale = MathUtil::Lerp(kDefeatedPopScale, 1.0f, eased);

	//後ろに帯を敷いて、背景が明るくても読めるようにする
	const int bandTop = kDefeatedPosY - kDefeatedBandHeight / 2;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(kDefeatedBandAlpha * eased));
	DrawBox(0, bandTop, Game::kScreenWidth, bandTop + kDefeatedBandHeight, Color::kBlack, true);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, ToAlpha(eased));
	FontManager::Instance().DrawCenteredExtendText(Game::kScreenWidth / 2, kDefeatedPosY, kDefeatedText,
		Color::kGold, kDefeatedFontSize, Color::kBlack, scale);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
