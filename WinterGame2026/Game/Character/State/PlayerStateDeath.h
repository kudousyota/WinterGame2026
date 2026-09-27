#pragma once
#include "PlayerStateBase.h"

//プレイヤーの死亡状態
//HPが0以下になったときにPlayerから強制的に遷移させる。ここから他のステートには遷移しない
class PlayerStateDeath : public PlayerStateBase
{
public:
	using PlayerStateBase::PlayerStateBase;

	void Update() override;
	StateKind GetKind() const override { return StateKind::Death; }
};
