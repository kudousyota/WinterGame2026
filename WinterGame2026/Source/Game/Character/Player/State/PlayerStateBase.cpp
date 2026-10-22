#include "PlayerStateBase.h"
#include "PlayerStateAttack.h"
#include "PlayerStateJump.h"
#include "PlayerStateDodge.h"
#include "../../../../System/Input.h"

Vector3 PlayerStateBase::GetMoveInput() const
{
	return m_player.GetMoveInput();
}

void PlayerStateBase::Move(float speedScale)
{
	auto move = GetMoveInput() * (m_player.GetSpeed() * speedScale);
	auto vel = m_player.GetVelocity();
	m_player.SetVelocity(Vector3(move.x, vel.y, move.z));
}

void PlayerStateBase::StopMove()
{
	m_player.StopMove();
}

void PlayerStateBase::ChangeAttackState(AttackType type, const char* buttonName)
{
	m_player.ChangeState(std::make_unique<PlayerStateAttack>(m_player, type,buttonName));
}

bool PlayerStateBase::TryGroundAction(int bufferFrame)
{
	auto& input = Input::Instance();

	//ジャンプボタンが押されたらJumpにする
	if (input.IsBuffered("Jump",bufferFrame))
	{
		//ジャンプに使ったので消費する
		input.ConsumeBuffer("Jump");
		m_player.ChangeState(std::make_unique<PlayerStateJump>(m_player, true));
		return true;
	}

	//回避ボタンが押されたらDodgeにする
	if (input.IsBuffered("Dodge", bufferFrame))
	{
		//回避に使ったので消費する
		input.ConsumeBuffer("Dodge");
		ChangeState<PlayerStateDodge>();
		return true;
	}

	//パンチボタンが押されたらパンチコンボの1段目を出す
	if (input.IsBuffered("Punch", bufferFrame))
	{
		//1段目に使った入力を消して、2段目が勝手に出ないようにする
		input.ConsumeBuffer("Punch");
		ChangeAttackState(AttackType::Punch,"Punch");
		return true;
	}

	//キックボタンが押されたらキックコンボの1段目を出す
	if (input.IsBuffered("Kick", bufferFrame))
	{
		//1段目に使った入力を消して、2段目が勝手に出ないようにする
		input.ConsumeBuffer("Kick");
		ChangeAttackState(AttackType::Kick,"Kick");
		return true;
	}

	return false;
}
