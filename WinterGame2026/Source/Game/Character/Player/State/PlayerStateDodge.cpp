#include "PlayerStateDodge.h"
#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"

namespace
{
	//回避にかかるフレーム数
	constexpr int kDodgeFrame = 20;
	//回避中の移動速度の倍率
	constexpr float kDodgeSpeedScale = 3.0f;
	//ジャスト回避を受け付けるフレーム数(回避の出始め)
	constexpr int kJustDodgeFrame = 8;
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
	//回避の出始めはジャスト回避を受け付ける
	m_player.SetJustDodgeWindow(true);
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

	//受付時間を過ぎたらジャスト回避は成立しない(無敵は続く)
	if (m_frame >= kJustDodgeFrame)
	{
		m_player.SetJustDodgeWindow(false);
	}
}

void PlayerStateDodge::Exit()
{
	//無敵を解除する
	m_player.SetInvincible(false);
	m_player.SetJustDodgeWindow(false);
}
