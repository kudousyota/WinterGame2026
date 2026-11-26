#include "PlayerHPUI.h"
#include "DxLib.h"
#include "../Game/Character/Character.h"

namespace
{
	//UI全体の大きさの倍率//これを変えると全部のパーツが同じ比率で大きさと間隔を変える
	constexpr float kScale = 0.7f;

	//基準点の画面上の座標//ゲージの左上をここに合わせる
	//ここを変えるとUI全体が動く
	constexpr int kBaseX = 70;
	constexpr int kBaseY = 75;

	//ここから下は倍率が1のときの値
	//座標は基準点からのずらし量

	//画像は回さない
	constexpr float kAngle = 0.0f;

	//ゲージ//左上の位置と満タンのときの大きさ
	constexpr float kGaugeOffsetX = 0.0f;
	constexpr float kGaugeOffsetY = 0.0f;
	constexpr float kGaugeWidth = 360.0f;
	constexpr float kGaugeHeight = 20.0f;

	//後ろの枠//DrawRotaGraphで描くので中心の位置
	constexpr float kBackOffsetX = 190.0f;
	constexpr float kBackOffsetY = 0.0f;
	constexpr float kBackScale = 0.8f;

	//蓋//DrawRotaGraphで描くので中心の位置
	//ゲージの両端に置くので、右の蓋はゲージの幅から決める
	constexpr float kLidOffsetX = 5.0f;
	constexpr float kLidOffsetY = 10.0f;
	constexpr float kLidScale = 1.0f;
}

PlayerHPUI::PlayerHPUI(const Character& player)
{
	m_pPlayer = &player;
}

PlayerHPUI::~PlayerHPUI()
{
	DeleteGraph(m_gaugeHandle);
	DeleteGraph(m_trailHandle);
	DeleteGraph(m_gaugeBackHandle);
	DeleteGraph(m_lidHandle);
}

void PlayerHPUI::Init()
{
	m_x = kBaseX;
	m_y = kBaseY;

	m_gaugeHandle = LoadGraph("Data/Image/gauge_green.png");
	m_trailHandle = HPGauge::LoadTrailGraph();
	m_gaugeBackHandle = LoadGraph("Data/Image/HPberfrem.png");
	m_lidHandle = LoadGraph("Data/Image/HPlid.png");
}

void PlayerHPUI::Update(Input& input)
{
	//割合の計算はHPGaugeに任せる
	m_gauge.Update(m_pPlayer->GetHp(), m_pPlayer->GetMaxHp());
}

void PlayerHPUI::Draw()
{
	//基準点//小数で計算するのでfloatにしておく
	const float baseX = static_cast<float>(m_x);
	const float baseY = static_cast<float>(m_y);

	//後ろの枠
	//位置のずらし量と画像の拡大率の両方に倍率を掛ける
	DrawRotaGraphF(baseX + kBackOffsetX * kScale, baseY + kBackOffsetY * kScale, kBackScale * kScale, kAngle, m_gaugeBackHandle, true);

	//ゲージ//減った分の赤を先に描いて、緑で上から隠す
	const float gaugeX = baseX + kGaugeOffsetX * kScale;
	const float gaugeY = baseY + kGaugeOffsetY * kScale;
	m_gauge.DrawTrail(gaugeX, gaugeY, kGaugeWidth * kScale, kGaugeHeight * kScale, m_trailHandle);
	m_gauge.Draw(gaugeX, gaugeY, kGaugeWidth * kScale, kGaugeHeight * kScale, m_gaugeHandle);

	//左の蓋
	const float lidY = baseY + kLidOffsetY * kScale;
	DrawRotaGraphF(baseX + kLidOffsetX * kScale, lidY, kLidScale * kScale, kAngle, m_lidHandle, true);
	//右の蓋//ゲージの右端に合わせる
	DrawRotaGraphF(baseX + (kGaugeWidth + kLidOffsetX) * kScale, lidY, kLidScale * kScale, kAngle, m_lidHandle, true);
}
