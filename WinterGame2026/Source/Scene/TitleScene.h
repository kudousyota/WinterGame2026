#pragma once
#include "Scene.h"
#include "../Game/Stage.h"

//タイトル画面
//決定ボタンでセレクト画面へ進む
class TitleScene : public Scene
{
public:
	TitleScene(SceneController& controller);
	~TitleScene() override;

	void Init() override;
	void Update() override;
	void Draw() override;

private:
	//案内文の点滅に使うフレーム数
	int m_frameCount = 0;
	//タイトルロゴの画像
	int m_logoHandle = -1;
	//背景に映すステージ//見るだけなので当たり判定は作らない
	Stage m_stage;
};
