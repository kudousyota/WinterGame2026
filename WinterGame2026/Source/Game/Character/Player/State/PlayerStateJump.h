#pragma once
#include "PlayerStateBase.h"

//プレイヤーのジャンプ状態
class PlayerStateJump : public PlayerStateBase
{
public:
	PlayerStateJump(Player& player, bool isJump);

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Jump; }
private:
	bool m_isJump = false;
};
