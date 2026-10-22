#include "PlayerStateWalk.h"
#include "PlayerStateIdle.h"
#include "PlayerStateJump.h"

namespace
{
	//先行入力を覚えておくフレーム数
	constexpr int kInputBufferFrame = 8;
}

void PlayerStateWalk::Update()
{
	//地面から離れたら落下にする
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

	//移動入力がなくなったらその場で止まってIdleにする
	if (GetMoveInput().SqMagnitude() <= 0.0f)
	{
		StopMove();
		ChangeState<PlayerStateIdle>();
		return;
	}

	Move(4.0f);
}
void PlayerStateWalk::Enter()
{
	m_player.GetAnim().ChangeAnim("Player|Run", true);
}