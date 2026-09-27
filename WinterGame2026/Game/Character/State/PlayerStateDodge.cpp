#include "PlayerStateDodge.h"
#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"

namespace
{
	//回避にかかるフレーム数
	constexpr int kDodgeFrame = 20;
	//回避中の移動速度の倍率
	constexpr float kDodgeSpeedScale = 3.0f;
}

void PlayerStateDodge::Enter()
{
	//入力方向に回避する。入力がなければ向いている方向に回避する
	auto input = GetMoveInput();
	if (input.SqMagnitude() > 0.0f)
	{
		m_dir = input.Normalize();
	}
	else
	{
		m_dir = m_player.GetForward();
	}

	//回避中は無敵
	m_player.SetInvincible(true);
}

void PlayerStateDodge::Update()
{
	//回避が終わったら入力に応じてIdleかWalkにする
	if (m_frame >= kDodgeFrame)
	{
		if (GetMoveInput().SqMagnitude() > 0.0f)
		{
			ChangeState<PlayerStateWalk>();
		}
		else
		{
			StopMove();
			ChangeState<PlayerStateIdle>();
		}
		return;
	}

	//回避方向に移動する
	auto move = m_dir * (m_player.GetSpeed() * kDodgeSpeedScale);
	auto vel = m_player.GetVelocity();
	m_player.SetVelocity(Vector3(move.x, vel.y, move.z));

	m_frame++;
}

void PlayerStateDodge::Exit()
{
	//無敵を解除する
	m_player.SetInvincible(false);
}
