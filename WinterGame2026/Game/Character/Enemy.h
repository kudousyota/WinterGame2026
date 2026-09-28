#pragma once
#include "Character.h"
class Enemy : public Character
{
public:
	Enemy();
	~Enemy()override;
	void Init()override;
	void Update()override;
	void Draw()override;
};

