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
		//押したボタンによって次の攻撃が変わる(後から押した方を優先)
		if (m_info.nextPunch && input.IsTriggered("Punch"))
		{
			m_reservedNext = m_info.nextPunch;
		}
		if (m_info.nextKick && input.IsTriggered("Kick"))
		{
			m_reservedNext = m_info.nextKick;
		}
	}

	//次の攻撃が予約されていたら、構えに戻るモーションを待たずに次の攻撃にする
	if (m_reservedNext && m_frame >= m_info.cancelFrame)
	{
		ChangeAttackState(*m_reservedNext);
		return;
	}

	//攻撃が終わったらIdleかWalkにする
	if (m_frame >= m_info.totalFrame)
	{
		if (GetMoveInput().SqMagnitude() > 0.0f)
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
