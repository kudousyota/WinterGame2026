#include "PlayerStateWalk.h"
#include "PlayerStateIdle.h"
#include "PlayerStateJump.h"
#include "PlayerStateDodge.h"
#include "../../../System/Input.h"

void PlayerStateWalk::Update()
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

	//移動入力がなくなったらその場で止まってIdleにする
	if (GetMoveInput().SqMagnitude() <= 0.0f)
	{
		StopMove();
		ChangeState<PlayerStateIdle>();
		return;
	}

	Move(1.0f);
}
