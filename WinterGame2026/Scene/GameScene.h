#pragma once
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
};

