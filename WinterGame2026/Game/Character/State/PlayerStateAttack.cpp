#include "PlayerStateAttack.h"
#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"
#include "PlayerStateDodge.h"
#include "../../../System/Input.h"
#include "../../../DataLoader/DataManager.h"

PlayerStateAttack::PlayerStateAttack(Player& player, AttackType type) :
	PlayerStateBase(player),
	m_info(DataManager::GetInstance().GetAttackTable().GetInfo(type))
{
}

void PlayerStateAttack::Enter()
{
	//攻撃中はその場で止まる
	StopMove();
	m_player.GetAnim().ChangeAnim(m_info.animName, false);
}

void PlayerStateAttack::Update()
{
	auto& input = Input::Instance();

	//受付フレームを過ぎたら次の攻撃の入力と回避キャンセルを受け付ける
	if (m_frame >= m_info.comboStartFrame)
	{
		if (input.IsTriggered("Dodge"))
		{
			ChangeState<PlayerStateDodge>();
			return;
		}
		if (m_info.next && input.IsTriggered("Attack"))
		{
			m_isComboReserved = true;
		}
	}

	//攻撃が終わったら次の攻撃かIdleかWalkにする
	if (m_frame >= m_info.totalFrame)
	{
		if (m_isComboReserved)
		{
			ChangeAttackState(*m_info.next);
		}
		else if (GetMoveInput().SqMagnitude() > 0.0f)
		{
			ChangeState<PlayerStateWalk>();
		}
		else
		{
			ChangeState<PlayerStateIdle>();
		}
		return;
	}

	if (IsHitActive())
	{
		//TODO:当たり判定ができたらここで m_info.damage と m_info.radius を使って判定する
	}

	m_frame++;
}

bool PlayerStateAttack::IsHitActive() const
{
	return m_info.hitStartFrame <= m_frame && m_frame < m_info.hitEndFrame;
}
