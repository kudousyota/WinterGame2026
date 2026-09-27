#pragma once
#include "Character.h"
class Player : public Character
{
public:
	Player();
	~Player()override;

	void Init()override;
	void Update()override;
	void Draw()override;
private:
	int m_modelHandle = -1;
};

