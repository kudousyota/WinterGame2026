#pragma once
#include <memory>
#include "../Game/Character/Player.h"
#include "../Game/Camera.h"

class GameScene
{
public:
	GameScene();
	~GameScene();

	void Init();
	void Update();
	void Draw();
private:
	void DrawGrid();

private:
	int m_frameCount;

	std::shared_ptr<Player> m_pPlayer;
	std::shared_ptr<Camera>m_pCamera;
};

