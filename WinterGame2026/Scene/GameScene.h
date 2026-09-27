#pragma once
#include <memory>
#include "../Game/Character/Player.h"
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
};

