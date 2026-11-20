#pragma once
#include "Scene.h"

class GameOverScene : public Scene
{
public:
	GameOverScene(SceneController& controller);

	void Init() override;
	void Update() override;
	void Draw() override;

private:
	//案内文の点滅に使うフレーム数
	int m_frameCount = 0;
};
