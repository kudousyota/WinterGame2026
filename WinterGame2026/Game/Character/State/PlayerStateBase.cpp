#include "PlayerStateBase.h"
#include "../../../System/Input.h"

Vector3 PlayerStateBase::GetMoveInput() const
{
	auto& input = Input::Instance();

	//左スティックの入力
	Vector3 dir(input.GetStickLX(), 0.0f, input.GetStickLY());

	//十字キーの入力
	if (input.IsPressed("right")) dir.x += 1.0f;
	if (input.IsPressed("left"))  dir.x -= 1.0f;
	if (input.IsPressed("up"))    dir.z += 1.0f;
	if (input.IsPressed("down"))  dir.z -= 1.0f;

	//斜め入力で速くならないよう長さを1以下に抑える
	if (dir.SqMagnitude() > 1.0f)
	{
		dir = dir.Normalize();
	}
	return dir;
}

void PlayerStateBase::Move(float speedScale)
{
	auto move = GetMoveInput() * (m_player.GetSpeed() * speedScale);
	auto vel = m_player.GetVelocity();
	m_player.SetVelocity(Vector3(move.x, vel.y, move.z));
}

void PlayerStateBase::StopMove()
{
	auto vel = m_player.GetVelocity();
	m_player.SetVelocity(Vector3(0.0f, vel.y, 0.0f));
}
