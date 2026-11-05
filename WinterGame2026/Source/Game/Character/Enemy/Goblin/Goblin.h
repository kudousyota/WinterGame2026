#pragma once
#include "../Enemy.h"

//近づいてきて腕を振る地上の雑魚敵
class Goblin : public Enemy
{
public:
	Goblin();
	~Goblin()override;
	void Init()override;
	void Update()override;
	void LateUpdate()override;
	void Draw()override;

private:
	int m_modelHandle = -1;
};
