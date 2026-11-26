#include "BossHPUI.h"
#include <algorithm>
#include "DxLib.h"
#include "../Game/GameManager.h"
#include "../Game/Character/Enemy/Enemy.h"
#include "../Common/FontManager.h"
#include "../Game.h"
#include "../Math/MathUtil.h"

namespace
{
	using Phase = BossCutscene::Phase;

	//ここから下は枠の画像の中のピクセル位置
	//枠の画像は1280x720で、まわりが透明になっている//雑魚のHPバーと同じ画像
	//左右の飾りはそのままの形で描き、真ん中の棒だけ横に伸ばす
	//飾りごと縮めると溝が太くなりすぎるので3つに分けて描く

	//使う範囲の上端と下端//飾りの上と下が入る高さ
	constexpr int kSrcTop = 266;
	constexpr int kSrcBottom = 380;
	//左の飾りの左端と、右の飾りの右端
	constexpr int kSrcLeft = 131;
	constexpr int kSrcRight = 1231;
	//真ん中の棒の、横に伸ばしてよい範囲//ここは縦に見るとどこも同じ模様
	constexpr int kSrcStretchLeft = 400;
	constexpr int kSrcStretchRight = 960;
	//ゲージを入れる溝//半透明の暗い色なのでゲージの背景にもなる
	constexpr int kSrcInnerLeft = 263;
	constexpr int kSrcInnerRight = 1099;
	constexpr int kSrcInnerTop = 289;
	constexpr int kSrcInnerBottom = 359;
	//ゲージの画像は上下に黒い縁があるので、枠の黒い縁に重なるよう溝より少し大きく描く
	constexpr int kSrcGaugeTop = 279;
	constexpr int kSrcGaugeBottom = 369;

	//枠の画像を描く倍率//飾りの大きさと溝の太さが変わる
	constexpr float kScale = 0.3f;

	//溝の画面上の大きさ//横は画面の半分くらい//縦は倍率で決まる
	constexpr float kInnerWidth = 720.0f;
	constexpr float kInnerHeight = (kSrcInnerBottom - kSrcInnerTop) * kScale;
	//溝の左上//横は真ん中に揃える//縦は画面の下からの距離で決める
	constexpr float kInnerLeft = (Game::kScreenWidth - kInnerWidth) * 0.5f;
	constexpr float kInnerTop = Game::kScreenHeight - 56.0f;

	//名前//ゲージの左上に小さく出す
	const char* const kBossName = "WARROK";
	constexpr int kNameFontSize = 24;
	//溝の上端から名前の縦の中心までの距離
	constexpr float kNameOffsetY = 22.0f;

	//出てきたときにゲージが伸びきるまでのフレーム数
	constexpr int kFillFrame = 60;
	//出るとき、消えるときに濃さが変わる速さ(1フレームあたり)
	constexpr float kAlphaSpeed = 0.08f;

	//遅れゲージ//ダメージを受けてから減り始めるまでのフレーム数と、減る速さ(1フレームあたり)
	constexpr int kTrailWaitFrame = 40;
	constexpr float kTrailSpeed = 0.006f;

	//ゲージを出す段階//カメラを戻している間から出し始めて、戻りきる頃には伸び始めている
	bool IsShowPhase(Phase phase)
	{
		return phase == Phase::None || phase == Phase::IntroReturn;
	}
}

BossHPUI::BossHPUI(const GameManager& game)
{
	m_pGame = &game;
}

BossHPUI::~BossHPUI()
{
	DeleteGraph(m_gaugeHandle);
	DeleteGraph(m_frameHandle);
}

void BossHPUI::Init()
{
	m_isVisible = true;
	m_gaugeHandle = LoadGraph("Data/Image/EnemyHP.png");
	m_frameHandle = LoadGraph("Data/Image/EnemyHPBer.png");
}

void BossHPUI::Update(Input& input)
{
	const BossCutscene& cutscene = m_pGame->GetBossCutscene();
	const std::shared_ptr<const Enemy> boss = cutscene.GetBoss();

	//ボスがいないときは次に出てきたときに伸ばし直せるよう戻しておく
	if (!boss)
	{
		m_alphaRate = (std::max)(m_alphaRate - kAlphaSpeed, 0.0f);
		if (m_alphaRate <= 0.0f)
		{
			m_fillRate = 0.0f;
			m_trailRate = 0.0f;
		}
		return;
	}

	//倒されたら撃破の演出に場所を譲る
	const bool isShown = IsShowPhase(cutscene.GetPhase()) && !boss->IsDead();
	if (isShown)
	{
		m_alphaRate = (std::min)(m_alphaRate + kAlphaSpeed, 1.0f);
		m_fillRate = (std::min)(m_fillRate + 1.0f / kFillFrame, 1.0f);
	}
	else
	{
		m_alphaRate = (std::max)(m_alphaRate - kAlphaSpeed, 0.0f);
	}

	const float prevRate = m_gauge.GetRate();
	m_gauge.Update(boss->GetHp(), boss->GetMaxHp());
	//減ったらしばらく遅れゲージを止めて、どれだけ削れたか見せる
	if (m_gauge.GetRate() < prevRate)
	{
		m_trailWaitFrame = kTrailWaitFrame;
	}

	//伸びている途中は遅れゲージも一緒に伸ばす
	const float shownRate = (std::min)(m_gauge.GetRate(), MathUtil::EaseOutCubic(m_fillRate));
	if (m_trailRate <= shownRate)
	{
		m_trailRate = shownRate;
	}
	else if (m_trailWaitFrame > 0)
	{
		m_trailWaitFrame--;
	}
	else
	{
		m_trailRate = (std::max)(m_trailRate - kTrailSpeed, shownRate);
	}
}

void BossHPUI::Draw()
{
	if (!m_isVisible) return;
	if (m_alphaRate <= 0.0f) return;

	const int alpha = static_cast<int>(255.0f * m_alphaRate);
	const float shownRate = (std::min)(m_gauge.GetRate(), MathUtil::EaseOutCubic(m_fillRate));

	//大きく縮めるのでギザギザにならないようにバイリニアで描く
	SetDrawMode(DX_DRAWMODE_BILINEAR);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

	DrawFrame();

	//遅れゲージ//溝の中に赤で描いて、本当のゲージより先の削れた分だけ見える
	if (m_trailRate > shownRate)
	{
		DrawBoxAA(kInnerLeft, kInnerTop, kInnerLeft + kInnerWidth * m_trailRate, kInnerTop + kInnerHeight, GetColor(220, 40, 40), true);
	}

	//本当のゲージ//伸びている途中は短く描く
	const float gaugeTop = kInnerTop + (kSrcGaugeTop - kSrcInnerTop) * kScale;
	const float gaugeBottom = kInnerTop + (kSrcGaugeBottom - kSrcInnerTop) * kScale;
	DrawExtendGraphF(kInnerLeft, gaugeTop, kInnerLeft + kInnerWidth * shownRate, gaugeBottom, m_gaugeHandle, true);

	SetDrawMode(DX_DRAWMODE_NEAREST);

	//名前
	FontManager::Instance().DrawLeftText(static_cast<int>(kInnerLeft), static_cast<int>(kInnerTop - kNameOffsetY),
		kBossName, GetColor(255, 220, 80), kNameFontSize, GetColor(0, 0, 0));
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void BossHPUI::DrawFrame() const
{
	//溝の左端と右端を画面上の位置に合わせて、そこから飾りの分だけ外へ広げる
	const float innerRight = kInnerLeft + kInnerWidth;
	const float left = kInnerLeft - (kSrcInnerLeft - kSrcLeft) * kScale;
	const float stretchLeft = kInnerLeft + (kSrcStretchLeft - kSrcInnerLeft) * kScale;
	const float stretchRight = innerRight - (kSrcInnerRight - kSrcStretchRight) * kScale;
	const float right = innerRight + (kSrcRight - kSrcInnerRight) * kScale;
	const float top = kInnerTop - (kSrcInnerTop - kSrcTop) * kScale;
	const float bottom = kInnerTop + (kSrcBottom - kSrcInnerTop) * kScale;
	const int srcHeight = kSrcBottom - kSrcTop;

	//左の飾り//そのままの比率
	DrawRectExtendGraphF(left, top, stretchLeft, bottom,
		kSrcLeft, kSrcTop, kSrcStretchLeft - kSrcLeft, srcHeight, m_frameHandle, true);
	//真ん中の棒//横にだけ伸ばす
	DrawRectExtendGraphF(stretchLeft, top, stretchRight, bottom,
		kSrcStretchLeft, kSrcTop, kSrcStretchRight - kSrcStretchLeft, srcHeight, m_frameHandle, true);
	//右の飾り//そのままの比率
	DrawRectExtendGraphF(stretchRight, top, right, bottom,
		kSrcStretchRight, kSrcTop, kSrcRight - kSrcStretchRight, srcHeight, m_frameHandle, true);
}
