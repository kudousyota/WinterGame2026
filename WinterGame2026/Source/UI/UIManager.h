#pragma once
#include <vector>
#include <memory>
#include "UIBase.h"
//UIをまとめて管理するクラス
//継承はせずにUIbaseのインスタンスとして保持する
class UIManager
{
public:
	UIManager() = default;
	~UIManager() = default;

	void Add(std::unique_ptr<UIBase> ui);
	void Init();
	void Update(Input& input);
	void Draw();
private:
	//管理しているUI
	//unique_ptr<UIBase>だから中身はHPUIでもWitchTimeNeedleUIでも何でも入る
	std::vector<std::unique_ptr<UIBase>> m_pUiList;
};

