#pragma once
#include "UIBase.h"
#include "HPGauge.h"

class Character;
class PlayerHPUI : public UIBase
{
public:
	//player:HPを見るキャラクター//GameManagerが持っていてUIより長く残るので借りるだけにする
	explicit PlayerHPUI(const Character& player);
	~PlayerHPUI();
	void Init()override;
	void Update(Input& input)override;
	void Draw()override;

private:
	//HPを見るプレイヤー//所有しない//読むだけなのでconst
	const Character* m_pPlayer = nullptr;

	//HPのゲージ//割合の計算とゲージの描画を任せる
	HPGauge m_gauge;

	//緑のゲージの画像ハンドル//HPGaugeに貸す
	int m_gaugeHandle = -1;
	//減った分を見せる赤いゲージの画像ハンドル//HPGaugeに貸す
	int m_trailHandle = -1;
	//後ろの枠の画像ハンドル
	int m_gaugeBackHandle = -1;
	//ゲージの両端の蓋の画像ハンドル
	int m_lidHandle = -1;
};
