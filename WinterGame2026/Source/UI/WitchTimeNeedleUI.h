#pragma once
#include "UIBase.h"

//ウィッチタイムに入った瞬間に画面の真ん中で針を回す演出
//大きく現れて勢いよく回り、だんだんゆっくりになって真上で止まってから消える
class WitchTimeNeedleUI: public UIBase
{
public:
	WitchTimeNeedleUI();
	~WitchTimeNeedleUI();
	void Init()override;
	void Update(Input& input)override;
	void Draw()override;
private:
	//針を1本描く//加算で光らせるので、大きさと濃さを変えて重ねるとぼんやり光って見える
	void DrawNeedle(float scale, float angle, int alpha)const;

	//針の画像ハンドル
	int m_needleHandle = -1;
	//演出を始めてからのフレーム数//演出中でないときは-1
	int m_frame = -1;
	//前のフレームでウィッチタイムだったか
	bool m_isPrevWitchTime = false;
};
