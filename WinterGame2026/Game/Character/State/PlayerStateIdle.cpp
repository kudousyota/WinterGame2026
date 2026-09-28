#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"
#include "PlayerStateJump.h"
#include "PlayerStateDodge.h"
#include "../../../System/Input.h"

void PlayerStateIdle::Update()
{
	auto& input = Input::Instance();

	//ジャンプボタンが押されたらJumpにする
	if (input.IsTriggered("Jump"))
	{
		ChangeState<PlayerStateJump>();
		return;
	}

	//回避ボタンが押されたらDodgeにする
	if (input.IsTriggered("Dodge"))
	{
		ChangeState<PlayerStateDodge>();
		return;
	}

	//移動入力があればWalkにする
	if (GetMoveInput().SqMagnitude() > 0.0f)
	{
		ChangeState<PlayerStateWalk>();
		return;
	}

	StopMove();
}

void PlayerStateIdle::Enter()
{
	m_player.GetAnim().ChangeAnim("Player|Idle", true);
}
