#include "PlayerStateDamage.h"
#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"
#include "PlayerStateJump.h"

namespace
{
	//被弾のアニメーション
	const char* const kDamageAnimName = "Player|Hit";
}

void PlayerStateDamage::Enter()
{
	//のけぞり中はその場で止まる(落下の速度は残す)
	StopMove();
	m_player.GetAnim().ChangeAnim(kDamageAnimName, false);
}

void PlayerStateDamage::Update()
{
	StopMove();

	//のけぞりのアニメーションが終わるまでは動けない
	if (!m_player.GetAnim().GetAnimEndFlag()) return;

	//空中ならそのまま落下にする
	if (!m_player.IsGround())
	{
		m_player.ChangeState(std::make_unique<PlayerStateJump>(m_player, false));
		return;
	}

	//地上なら入力に応じてIdleかWalkにする
	if (GetMoveInput().SqMagnitude() > 0.0f)
	{
		ChangeState<PlayerStateWalk>();
	}
	else
	{
		ChangeState<PlayerStateIdle>();
	}
}