#pragma once
#include "PlayerStateBase.h"

//プレイヤーの待機状態
class PlayerStateIdle : public PlayerStateBase
{
public:
	//usingでplayerを受け取って親に渡す
	using PlayerStateBase::PlayerStateBase;

	void Update() override;
	StateKind GetKind() const override { return StateKind::Idle; }
};
