#include "HPGauge.h"
#include <algorithm>
#include "DxLib.h"

namespace
{
	//赤いゲージ//減ってから減り始めるまでのフレーム数と、1フレームに減る割合
	//ボスのHPバーと同じ値にして見た目をそろえる
	constexpr int kTrailWaitFrame = 40;
	constexpr float kTrailSpeed = 0.006f;

	//赤いゲージの画像//1200x600の透明の中に、幅1の帯が縦に入っている
	const char* const kTrailPath = "Data/Image/gauge_red.png";
	//帯の左上と大きさ//画像を描き直したらここも合わせる
	constexpr int kTrailSrcX = 450;
	constexpr int kTrailSrcY = 303;
	constexpr int kTrailSrcWidth = 1;
	constexpr int kTrailSrcHeight = 22;
}

void HPGauge::Update(int current, int max)
{
	const float prevRate = m_rate;

	//最大HPが0だと割れないのでゲージを空にする
	if (max <= 0)
	{
		m_rate = 0.0f;
	}
	else
	{
		//とどめの攻撃でHPがマイナスになることがあるので0から1に収める
		//マイナスの幅で描くと画像が反転して左にはみ出す
		m_rate = std::clamp(static_cast<float>(current) / static_cast<float>(max), 0.0f, 1.0f);
	}

	//減ったら待ち時間を最初からにする//連続で当てている間は赤が残り続ける
	if (m_rate < prevRate)
	{
		m_trailWaitFrame = kTrailWaitFrame;
	}

	//回復したときや最初のフレームは赤を残さずそろえる
	if (m_trailRate <= m_rate)
	{
		m_trailRate = m_rate;
	}
	else if (m_trailWaitFrame > 0)
	{
		m_trailWaitFrame--;
	}
	else
	{
		m_trailRate = (std::max)(m_trailRate - kTrailSpeed, m_rate);
	}
}

void HPGauge::Draw(float x, float y, float width, float height, int gaugeHandle, int alpha) const
{
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	//残りの割合の分だけ右端を縮める
	//小さく描いたときにカクつかないようにFの付いた関数で小数のまま描く
	DrawExtendGraphF(x, y, x + width * m_rate, y + height, gaugeHandle, true);
	//後に描くUIまで透けないように元に戻す
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void HPGauge::DrawTrail(float x, float y, float width, float height, int trailHandle, int alpha) const
{
	//本当のゲージより長いときだけ、はみ出した分が削れた分として見える
	if (m_trailRate <= m_rate) return;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawExtendGraphF(x, y, x + width * m_trailRate, y + height, trailHandle, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

int HPGauge::LoadTrailGraph()
{
	//大きな画像の一部を描くと、バイリニアのときにまわりの透明が混ざって薄くなる
	//帯だけを別の画像に写しておけば、緑や黄色のゲージと同じように引き延ばして描ける
	const int srcSoft = LoadSoftImage(kTrailPath);
	if (srcSoft == -1) return -1;

	const int dstSoft = MakeARGB8ColorSoftImage(kTrailSrcWidth, kTrailSrcHeight);
	BltSoftImage(kTrailSrcX, kTrailSrcY, kTrailSrcWidth, kTrailSrcHeight, srcSoft, 0, 0, dstSoft);
	const int handle = CreateGraphFromSoftImage(dstSoft);

	DeleteSoftImage(srcSoft);
	DeleteSoftImage(dstSoft);
	return handle;
}
