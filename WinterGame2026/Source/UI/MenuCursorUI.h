#pragma once
#include "UIBase.h"

//メニューで選んでいる項目の左に出す矢印
//ニードルの画像を横に倒して矢印にする//セレクト画面とポーズメニューで使う
class MenuCursorUI : public UIBase
{
public:
	MenuCursorUI() = default;
	~MenuCursorUI();
	void Init()override;
	void Update(Input& input)override;
	void Draw()override;

	//指す項目の文字の中心を渡す//矢印はその左に出す
	void SetItemPos(int x, int y);

private:
	//ニードルの画像
	int m_handle = -1;
	//左右に揺らすためのフレーム数
	int m_frame = 0;
};
