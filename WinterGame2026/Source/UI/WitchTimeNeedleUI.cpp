#include "WitchTimeNeedleUI.h"
#include "../System/Timer.h"
#include "../Game.h"
#include "../Math/MathUtil.h"
#include "DxLib.h"
#include <algorithm>

namespace
{
	//画像の大きさ//回す中心は画像の真ん中
	constexpr int kImageWidth = 256;
	constexpr int kImageHeight = 512;

	//演出全体のフレーム数
	constexpr int kTotalFrame = 70;
	//現れるのにかけるフレーム数//大きいところから縮みながら濃くなる
	constexpr int kAppearFrame = 8;
	//消えるのにかけるフレーム数//最後のこのフレーム数で薄くなりながら少し広がる
	constexpr int kDisappearFrame = 18;
	//回り終わるまでのフレーム数//ここで真上に止まり、消えるまで少し止まって見せる
	constexpr int kSpinFrame = 48;

	//回る量//2周して真上で止まる
	constexpr float kSpinAngle = DX_TWO_PI_F * 2.0f;

	//止まっているときの大きさ
	constexpr float kBaseScale = 1.4f;
	//現れた瞬間の大きさ//ここから止まっているときの大きさへ縮む
	constexpr float kAppearScale = 2.4f;
	//消えるときに広がる量
	constexpr float kDisappearScaleAdd = 0.3f;

	//後ろに重ねる光の大きさ(針に対する倍率)と濃さ
	constexpr float kGlowScaleRate = 1.15f;
	constexpr int kGlowAlpha = 90;
	//針そのものの濃さ
	constexpr int kNeedleAlpha = 255;
}

WitchTimeNeedleUI::WitchTimeNeedleUI()
{
}

WitchTimeNeedleUI::~WitchTimeNeedleUI()
{
	if (m_needleHandle != -1)
	{
		DeleteGraph(m_needleHandle);
	}
}

void WitchTimeNeedleUI::Init()
{
	m_needleHandle = LoadGraph("Data/Image/ui_niidle.png");
}

void WitchTimeNeedleUI::Update(Input& input)
{
	//敵がスロー中ならウィッチタイム
	const bool isWitchTime = Timer::Instance().IsEnemySlow();

	//ウィッチタイムに入った瞬間に最初から始める
	if (isWitchTime && !m_isPrevWitchTime)
	{
		m_frame = 0;
	}
	m_isPrevWitchTime = isWitchTime;

	//演出中でない
	if (m_frame < 0) return;

	//フレームはDrawではなくここで数える//デバッグの一時停止中に回らないようにする
	m_frame++;
	if (m_frame >= kTotalFrame)
	{
		m_frame = -1;
	}
}

void WitchTimeNeedleUI::Draw()
{
	//演出中でない、またはウィッチタイムが先に終わった
	if (m_frame < 0 || !Timer::Instance().IsEnemySlow()) return;

	const float frame = static_cast<float>(m_frame);

	//回る角度//最初は勢いよく、だんだんゆっくりになって真上で止まる
	const float spinRate = std::clamp(frame / kSpinFrame, 0.0f, 1.0f);
	const float angle = kSpinAngle * MathUtil::EaseOutCubic(spinRate);

	//大きさと濃さ
	float scale = kBaseScale;
	float fade = 1.0f;
	//現れるところ//大きいところからシュッと縮みながら濃くなる
	if (m_frame < kAppearFrame)
	{
		const float t = MathUtil::EaseOutCubic(frame / kAppearFrame);
		scale = MathUtil::Lerp(kAppearScale, kBaseScale, t);
		fade = t;
	}
	//消えるところ//薄くなりながら少し広がる
	const int disappearStart = kTotalFrame - kDisappearFrame;
	if (m_frame >= disappearStart)
	{
		const float t = static_cast<float>(m_frame - disappearStart) / kDisappearFrame;
		scale += kDisappearScaleAdd * t;
		fade = 1.0f - t;
	}

	//加算で描いて光らせる//暗いウィッチタイムの画面の上で針が光って見え、後ろのプレイヤーも透けて見える
	SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
	//後ろにひと回り大きく薄い針を重ねて、ぼんやりした光にする
	DrawNeedle(scale * kGlowScaleRate, angle, static_cast<int>(kGlowAlpha * fade));
	DrawNeedle(scale, angle, static_cast<int>(kNeedleAlpha * fade));
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void WitchTimeNeedleUI::DrawNeedle(float scale, float angle, int alpha) const
{
	if (alpha <= 0) return;
	//加算の強さで濃さを変える
	SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
	DrawRotaGraph2(Game::kScreenWidth / 2, Game::kScreenHeight / 2, kImageWidth / 2, kImageHeight / 2,
		scale, angle, m_needleHandle, true, false);
}
