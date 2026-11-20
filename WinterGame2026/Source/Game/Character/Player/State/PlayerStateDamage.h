#pragma once
#include "PlayerStateBase.h"


class PlayerStateDamage : public PlayerStateBase
{
public:
	//usingでplayerを受け取って親に渡す
	using PlayerStateBase::PlayerStateBase;

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Damage; }
};