#pragma once
#include "UIBase.h"
#include "HPGauge.h"

class GameManager;
class BossHPUI : public UIBase
{
public:
	//GameManagerはシーンが持っていてUIより長く残るので借りるだけにする
	explicit BossHPUI(const GameManager& game);
	~BossHPUI();

	void Init()override;
	void Update(Input& input)override;
	void Draw()override;

private:
	//枠//左右の飾りはそのままの比率で、真ん中の棒だけ横に伸ばして描く
	void DrawFrame()const;

	//ボスと演出の段階を見る//所有しない
	const GameManager* m_pGame = nullptr;
	//本当のHPの割合
	HPGauge m_gauge;

	//出てきたときにゲージを左から伸ばす割合//0～1//ボスがいなくなったら0に戻す
	float m_fillRate = 0.0f;
	//減った分を遅れて追いかけるゲージの割合//0～1
	float m_trailRate = 0.0f;
	//最後にダメージを受けてからのフレーム数//しばらくは遅れゲージを止めておく
	int m_trailWaitFrame = 0;
	//全体の濃さ//0～1
	float m_alphaRate = 0.0f;

	//ゲージの画像ハンドル//雑魚のバーと同じ色にする
	int m_gaugeHandle = -1;
	//枠の画像ハンドル//雑魚のバーと同じ飾りの枠
	int m_frameHandle = -1;
};
