#pragma once
#include "Character.h"

class Player : public Character
{
public:
	Player();
	~Player();

	void Init() override;
	void Update() override;
};
