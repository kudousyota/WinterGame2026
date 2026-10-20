#pragma once
#include "Scene.h"

//タイトル画面
//決定ボタンでゲームを始める
class TitleScene : public Scene
{
public:
	TitleScene(SceneController& controller);

	void Init() override;
	void Update() override;
	void Draw() override;

private:
	//案内文の点滅に使うフレーム数
	int m_frameCount = 0;
};
