#pragma once
#include "PlayerStateBase.h"

class PlayerStateDeath : public PlayerStateBase
{
public:
	using PlayerStateBase::PlayerStateBase;

	void Update() override;
	StateKind GetKind() const override { return StateKind::Death; }
};
