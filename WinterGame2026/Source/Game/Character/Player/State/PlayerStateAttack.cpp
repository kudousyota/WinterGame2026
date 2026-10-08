#include "PlayerStateAttack.h"
#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"
#include "PlayerStateDodge.h"
#include "../../../../System/Input.h"
#include "../../../../DataLoader/DataManager.h"

namespace
{
	//攻撃判定を出す位置(足元から前方への距離と高さ)
	//ボーンの位置から出すようにしたらCSVで攻撃ごとに指定する
	constexpr float kHitForwardOffset = 60.0f;
	constexpr float kHitHeight = 100.0f;
	//先行入力を覚えておくフレーム数
	constexpr int kInputBufferFrame = 8;
}

PlayerStateAttack::PlayerStateAttack(Player& player, AttackType type):
	PlayerStateBase(player),
	m_info(DataManager::GetInstance().GetAttackTable().GetInfo(type))
{
}

void PlayerStateAttack::Enter()
{
	//入力があればその方向に向く
	//入力がなければ今の向きを維持する
	auto dir = GetMoveInput();
	if (dir.SqMagnitude() > 0.0f)
	{
		//長さが1より短いと攻撃判定がずれるので正規化する
		m_player.FaceTo(dir.Normalize());
	}

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
		//受付前に押した回避もここで拾う(消費しないと攻撃後に遅れて回避が出る)
		if (input.IsBuffered("Dodge", kInputBufferFrame))
		{
			input.ConsumeBuffer("Dodge");
			ChangeState<PlayerStateDodge>();
			return;
		}
		//押したボタンによって次の攻撃が変わる(後から押した方を優先)
		//次の攻撃がないボタンは押されていない扱いにする
		std::optional<int> punchFrame;
		std::optional<int> kickFrame;
		if (m_info.nextPunch)
		{
			punchFrame = input.GetBufferedFrame("Punch", kInputBufferFrame);
		}
		if (m_info.nextKick)
		{
			kickFrame = input.GetBufferedFrame("Kick", kInputBufferFrame);
		}

		if (punchFrame || kickFrame)
		{
			//押したフレームが新しい方を選ぶ(同時押しはキックを優先)
			if (kickFrame && (!punchFrame || *kickFrame >= *punchFrame))
			{
				m_reservedNext = m_info.nextKick;
			}
			else
			{
				m_reservedNext = m_info.nextPunch;
			}
			//負けた方も消費しないと、次のフレームで予約が上書きされてしまう
			input.ConsumeBuffer("Punch");
			input.ConsumeBuffer("Kick");
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
		//攻撃を申請するだけ
		AttackRequest request{AttackData(CharacterType::Player, m_info.type, m_info.damage, m_info.radius)};
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
