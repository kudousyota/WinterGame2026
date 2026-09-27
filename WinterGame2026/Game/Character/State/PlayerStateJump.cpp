#include "PlayerStateJump.h"
#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"

namespace
{
	//ジャンプ力
	constexpr float kJumpPower = 12.0f;
	//空中で動ける速さの倍率
	constexpr float kAirMoveSpeedScale = 1.0f;
}

void PlayerStateJump::Enter()
{
	//このステートに入った瞬間に上方向の速度を与える
	auto vel = m_player.GetVelocity();
	vel.y = kJumpPower;
	m_player.SetVelocity(vel);
}

void PlayerStateJump::Update()
{
	//着地したら入力に応じてIdleかWalkにする
	if (m_player.IsGround())
	{
		if (GetMoveInput().SqMagnitude() > 0.0f)
		{
			ChangeState<PlayerStateWalk>();
		}
		else
		{
			ChangeState<PlayerStateIdle>();
		}
		return;
	}

	//空中でも移動できる
	Move(kAirMoveSpeedScale);
}
