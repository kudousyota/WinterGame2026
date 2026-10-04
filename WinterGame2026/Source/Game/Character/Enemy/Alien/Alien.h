#pragma once
#include "../Enemy.h"
class Alien : public Enemy
{
public:
	Alien();
	~Alien()override;
	void Init()override;
	void Update()override;
	void LateUpdate()override;
	void Draw()override;

private:
	int m_modelHandle = -1;
};