#pragma once
#include "UIBase.h"
class WitchTimeNeedleUI: public UIBase
{
public:
	WitchTimeNeedleUI();
	~WitchTimeNeedleUI();
	void Init()override;
	void Update(Input& input)override;
	void Draw()override;
private:
	//針の画像ハンドル
	int m_needleHandle = -1;
	//回転角度
	float m_angle = 0.0f;
	//演出の残りフレーム
	int m_witchEffectFrame = 0;
	//前のフレームでウィッチタイムだったか
	bool m_isPrevWitchTime = false;
};

