#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"
#include "PlayerStateJump.h"

namespace
{
	//先行入力を覚えておくフレーム数
	constexpr int kInputBufferFrame = 8;
}

void PlayerStateIdle::Update()
{
	if (!m_player.IsGround())
	{
		m_player.ChangeState(std::make_unique<PlayerStateJump>(m_player, false));
		return;
	}

	//ジャンプ・回避・攻撃の入力があればそちらにする
	if (TryGroundAction(kInputBufferFrame))
	{
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
