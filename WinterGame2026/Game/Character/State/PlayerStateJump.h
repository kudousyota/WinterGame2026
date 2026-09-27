#pragma once
#include "PlayerStateBase.h"

//プレイヤーのジャンプ状態
class PlayerStateJump : public PlayerStateBase
{
public:

	using PlayerStateBase::PlayerStateBase;

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Jump; }
};
