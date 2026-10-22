#include "PlayerStateJump.h"
#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"
#include "../../../../System/Input.h"

namespace
{
	//ジャンプ力
	constexpr float kJumpPower = 12.0f;
	//空中で動ける速さの倍率
	constexpr float kAirMoveSpeedScale = 1.0f;
	//先行入力を覚えておくフレーム数
	constexpr int kBufferFrame = 8;
}

PlayerStateJump::PlayerStateJump(Player& player, bool isJump):
PlayerStateBase(player)
{
	m_isJump = isJump;
}

void PlayerStateJump::Enter()
{
	//落下状態になったらアニメーションを変える
	m_player.GetAnim().ChangeAnim("Player|Jump", false);
	if (m_isJump)
	{
		//ジャンプ状態になったら上方向に速度を与える
		auto vel = m_player.GetVelocity();
		vel.y = kJumpPower;
		m_player.SetVelocity(vel);
	}
	else
	{
		//空中コンボの後、段差から落ちた時は浮ける権利を使い切る
		m_player.UseAirHover();
	}
}

void PlayerStateJump::Update()
{
	auto& input = Input::Instance();

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

	//パンチボタンが押されたらパンチコンボの1段目を出す
	if (input.IsBuffered("Punch", kBufferFrame))
	{
		//1段目に使った入力を消して、2段目が勝手に出ないようにする
		input.ConsumeBuffer("Punch");
		ChangeAttackState(AttackType::AirPunch, "Punch");
		return;
	}
	//キックボタンが押されたらキックコンボの1段目を出す
	if (input.IsBuffered("Kick", kBufferFrame))
	{
		//1段目に使った入力を消して、2段目が勝手に出ないようにする
		input.ConsumeBuffer("Kick");
		ChangeAttackState(AttackType::AirSkyKick, "Kick");
		return;
	}
	//空中でも移動できる
	Move(kAirMoveSpeedScale);
}
