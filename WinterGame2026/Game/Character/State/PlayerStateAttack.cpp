#include "PlayerStateAttack.h"
#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"
#include "PlayerStateDodge.h"
#include "../../../System/Input.h"
#include "../../../DataLoader/DataManager.h"

namespace
{
	//攻撃判定を出す位置(足元から前方への距離と高さ)
	//TODO:骨の位置から出すようにしたらCSVで攻撃ごとに指定する
	constexpr float kHitForwardOffset = 60.0f;
	constexpr float kHitHeight = 100.0f;
}

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
	//攻撃ごとに新しい番号をもらう
	m_attackId = m_player.IssueAttackId();
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
		//攻撃を申請するだけ(判定はGameSceneが全員の移動が終わった後に行う)
		AttackRequest request{ AttackData(CharacterType::Player, m_info.type, m_info.damage, m_info.radius) };
		request.attackId = m_attackId;
		request.forwardOffset = kHitForwardOffset;
		request.height = kHitHeight;
		m_player.RequestAttack(request);
	}

	m_frame++;
}

#ifdef _DEBUG
std::string PlayerStateAttack::GetDebugInfo() const
{
	char buf[128];
	sprintf_s(buf, "%s  Frame:%d/%d  Hit:%s  Id:%d",
		m_info.animName.c_str(), m_frame, m_info.totalFrame, IsHitActive() ? "ON" : "OFF", m_attackId);
	return buf;
}
#endif

bool PlayerStateAttack::IsHitActive() const
{
	return m_info.hitStartFrame <= m_frame && m_frame < m_info.hitEndFrame;
}
