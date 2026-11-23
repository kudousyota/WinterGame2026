#include "HPGauge.h"
#include <algorithm>
#include "DxLib.h"

void HPGauge::Update(int current, int max)
{
	//最大HPが0だと割れないのでゲージを空にする
	if (max <= 0)
	{
		m_rate = 0.0f;
		return;
	}

	//とどめの攻撃でHPがマイナスになることがあるので0から1に収める
	//マイナスの幅で描くと画像が反転して左にはみ出す
	m_rate = std::clamp(static_cast<float>(current) / static_cast<float>(max), 0.0f, 1.0f);
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
