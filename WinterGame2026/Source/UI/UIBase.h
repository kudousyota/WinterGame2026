#pragma once

class Input;
class UIBase
{
public:
	//純粋仮想関数
	virtual ~UIBase() = default;
	virtual void Init() = 0;
	virtual void Update(Input& input) = 0;
	virtual void Draw() = 0;
	//外部から表示状態を変更する
	void SetVisible(bool isVisible) { m_isVisible = isVisible; }
	//現在表示すべきかどうかを取得するため
	bool IsVisible()const { return m_isVisible; }

protected:
	//UIを表示するかどうか
	bool m_isVisible = false;
	int m_x = 0;
	int m_y = 0;
};

