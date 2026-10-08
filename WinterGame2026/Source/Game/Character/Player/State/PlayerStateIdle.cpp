#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"
#include "PlayerStateJump.h"
#include "PlayerStateDodge.h"
#include "../../../../System/Input.h"

namespace
{
	//先行入力を覚えておくフレーム数(Walk・Attackと同じ値にする)
	constexpr int kInputBufferFrame = 8;
}

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
	if (input.IsBuffered("Dodge", kInputBufferFrame))
	{
		//回避に使ったので消費する
		input.ConsumeBuffer("Dodge");
		ChangeState<PlayerStateDodge>();
		return;
	}

	//パンチボタンが押されたらパンチコンボの1段目を出す
	if (input.IsBuffered("Punch", kInputBufferFrame))
	{
		//1段目に使った入力を消して、2段目が勝手に出ないようにする
		input.ConsumeBuffer("Punch");
		ChangeAttackState(AttackType::Punch);
		return;
	}

	//キックボタンが押されたらキックコンボの1段目を出す
	if (input.IsBuffered("Kick", kInputBufferFrame))
	{
		//1段目に使った入力を消して、2段目が勝手に出ないようにする
		input.ConsumeBuffer("Kick");
		ChangeAttackState(AttackType::Kick);
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
