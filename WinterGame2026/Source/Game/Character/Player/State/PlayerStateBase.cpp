#include "PlayerStateBase.h"
#include "PlayerStateAttack.h"
#include "PlayerStateJump.h"
#include "PlayerStateDodge.h"
#include "../../../../System/Input.h"

Vector3 PlayerStateBase::GetMoveInput() const
{
	auto& input = Input::Instance();

	//左スティックの入力
	Vector3 dir(input.GetStickLX(), 0.0f, input.GetStickLY());
	//スティックが倒れていない時だけ十字キーを見る
	if (dir.SqMagnitude() <= 0.0f)
	{
		//十字キーの入力
		if (input.IsPressed("right")) dir.x += 1.0f;
		if (input.IsPressed("left"))  dir.x -= 1.0f;
		if (input.IsPressed("up"))    dir.z += 1.0f;
		if (input.IsPressed("down"))  dir.z -= 1.0f;
	}
	//斜め入力で速くならないよう長さを1以下に抑える
	if (dir.SqMagnitude() > 1.0f)
	{
		dir = dir.Normalize();
	}

	//入力をカメラ基準の方向に変換する
	return m_player.GetCameraRight() * dir.x + m_player.GetCameraForward() * dir.z;
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

void PlayerStateBase::ChangeAttackState(AttackType type)
{
	m_player.ChangeState(std::make_unique<PlayerStateAttack>(m_player, type));
}

bool PlayerStateBase::TryGroundAction(int bufferFrame)
{
	auto& input = Input::Instance();

	//ジャンプボタンが押されたらJumpにする
	if (input.IsTriggered("Jump"))
	{
		ChangeState<PlayerStateJump>();
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
		ChangeAttackState(AttackType::Punch);
		return true;
	}

	//キックボタンが押されたらキックコンボの1段目を出す
	if (input.IsBuffered("Kick", bufferFrame))
	{
		//1段目に使った入力を消して、2段目が勝手に出ないようにする
		input.ConsumeBuffer("Kick");
		ChangeAttackState(AttackType::Kick);
		return true;
	}

	return false;
}
