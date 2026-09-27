#pragma once
#include "PlayerStateBase.h"

//プレイヤーの移動状態
class PlayerStateWalk : public PlayerStateBase
{
public:
	using PlayerStateBase::PlayerStateBase;

	void Update() override;
	StateKind GetKind() const override { return StateKind::Walk; }
};
