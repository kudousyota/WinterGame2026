#include "PlayerStateAttack.h"
#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"
#include "PlayerStateDodge.h"
#include "../../../../System/Input.h"
#include "../../../../DataLoader/DataManager.h"
#include "../../../../Effect/EffectManager.h"
#include <algorithm>

namespace
{
	//攻撃判定を出す位置(足元から前方への距離と高さ)
	//ボーンの位置から出すようにしたらCSVで攻撃ごとに指定する
	constexpr float kHitForwardOffset = 60.0f;
	constexpr float kHitHeight = 100.0f;
	//先行入力を覚えておくフレーム数
	constexpr int kInputBufferFrame = 8;
	//長押しで撃つ弾
	constexpr int kShotInterval = 6;
	constexpr float kBulletSpeed = 30.0f;
	constexpr int kBulletDamage = 1;
	constexpr float kBulletRadius = 10.0f;
	//敵が遠いとき、前進距離をCSVの値の何倍まで伸ばすか
	constexpr float kMaxMoveScale = 2.0f;
	//敵の手前で止まるときに、体同士の間に空ける隙間
	constexpr float kStopMargin = 10.0f;
	//ウィケッドウィーブを出す位置(足元から前方への距離)
	constexpr float kWitchHandForwardOffset = 150.0f;
	//ウィケッドウィーブのエフェクトの拡大率(Effekseerの1単位を何cmにするか)
	constexpr float kWitchHandScale = 40.0f;
	//エフェクトの原点から拳(着弾点)までの距離(Effekseerの単位)
	constexpr float kWitchHandFistDistance = 1.1f;
	//魔法陣を出したい位置(足元から見て後ろ・上・右への距離)。右肩のあたりから前へ突き出す
	constexpr float kWitchHandPortalBack = 50.0f;
	constexpr float kWitchHandPortalHeight = 150.0f;
	constexpr float kWitchHandPortalSide = 40.0f;

	//最初が速く、だんだん遅くなる(0～1を受け取って0～1を返す)
	float EaseOut(float t)
	{
		return 1.0f - (1.0f - t) * (1.0f - t);
	}
}

PlayerStateAttack::PlayerStateAttack(Player& player, AttackType type, const char* buttonName):
	PlayerStateBase(player),
	m_info(DataManager::GetInstance().GetAttackTable().GetInfo(type)),
	m_buttonName(buttonName)
{
}

void PlayerStateAttack::Enter()
{
	//狙う相手がいなかったらCSVの値分進む
	m_moveDistance = m_info.moveDistance;

	//狙う相手がいればそちらを向く
	if (const Character* target = m_player.GetAttackTarget())
	{
		//敵への向き(上下は無視する)
		Vector3 toTarget = target->GetPos() - m_player.GetPos();
		toTarget.y = 0.0f;
		//重なっていると向きが決まらないので、そのときは向きを変えない
		if (toTarget.SqMagnitude() > 0.0f)
		{
			m_player.FaceTo(toTarget.Normalize());
		}

		//踏み込む攻撃だけ、敵との距離に合わせて進む距離を変える
		if (m_info.moveDistance > 0.0f)
		{
			//体同士が触れる少し前で止まる
			float stopDistance = m_player.GetCollisionRadius() + target->GetCollisionRadius() + kStopMargin;
			float maxDistance = m_info.moveDistance * kMaxMoveScale;
			//近すぎるときは進まない、遠すぎるときは最大距離まで
			m_moveDistance = std::clamp(toTarget.Magnitude() - stopDistance, 0.0f,maxDistance);
		}
	}
	//いなければ、入力があればその方向に向く
	else
	{
		auto dir = GetMoveInput();
		if (dir.SqMagnitude() > 0.0f)
		{
			//長さが1より短いと攻撃判定がずれるので正規化する
			m_player.FaceTo(dir.Normalize());
		}
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

	if (input.IsBuffered("Dodge", kInputBufferFrame))
	{
		input.ConsumeBuffer("Dodge");
		ChangeState<PlayerStateDodge>();
		return;
	}

	//受付フレームを過ぎたら次の攻撃の入力と回避キャンセルを受け付ける
	if (m_frame >= m_info.comboStartFrame)
	{
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
				m_reservedButton = "Kick";
			}
			else
			{
				m_reservedNext = m_info.nextPunch;
				m_reservedButton = "Punch";
			}
			//負けた方も消費しないと、次のフレームで予約が上書きされてしまう
			input.ConsumeBuffer("Punch");
			input.ConsumeBuffer("Kick");
		}
	}

	//次の攻撃が予約されていたら、構えに戻るモーションを待たずに次の攻撃にする
	if (m_reservedNext && m_frame >= m_info.cancelFrame)
	{
		ChangeAttackState(*m_reservedNext, m_reservedButton);
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

	//コンボの締めは、攻撃判定が出た瞬間にウィケッドウィーブを呼び出す
	if (m_info.witchHand && !m_witchHandInfo && m_frame == m_info.hitStartFrame)
	{
		SummonWitchHand();
	}
	//長押しで止まっている間も手の攻撃は進める
	UpdateWitchHand();

	//攻撃判定が終わってもボタンを押し続けていたら弾を出す
	bool isHold = m_frame >= m_info.hitEndFrame && input.IsPressed(m_buttonName) && !m_reservedNext;
	m_player.SetAnimPause(isHold);
	if (isHold)
	{
		//長押しは前進も止める
		StopMove();
		m_shotTimer--;
		if (m_shotTimer <= 0)
		{
			//弾を出す
			m_shotTimer = kShotInterval;
			ShotRequest request{AttackData(CharacterType::Player, m_info.type, kBulletDamage,kBulletRadius)};
			request.speed = kBulletSpeed;
			request.forwardOffset = kHitForwardOffset;
			request.height = kHitHeight;
			m_player.RequestShot(request);
		}
		//フレームを進めないでボタンを離すと続きから進む
		return;
	}
	
	//前進する
	UpdateMove();

	//多段ヒットの攻撃は、間隔ごとに新しい番号にして同じ相手にもう一度当たるようにする
	if (m_info.hitInterval > 0 && m_frame > m_info.hitStartFrame &&
		(m_frame - m_info.hitStartFrame) % m_info.hitInterval == 0)
	{
		m_attackId = m_player.IssueAttackId();
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

void PlayerStateAttack::Exit()
{
	m_player.SetAnimPause(false);
}

#ifdef _DEBUG
std::string PlayerStateAttack::GetDebugInfo() const
{
	char buf[160];
	sprintf_s(buf, "%s  Frame:%d/%d  Hit:%s  Id:%d  Move:%.0f",
		m_info.animName.c_str(), m_frame, m_info.totalFrame, IsHitActive() ? "ON" : "OFF", m_attackId, m_moveDistance);
	return buf;
}
#endif

bool PlayerStateAttack::IsHitActive() const
{
	return m_info.hitStartFrame <= m_frame && m_frame < m_info.hitEndFrame;
}

void PlayerStateAttack::SummonWitchHand()
{
	m_witchHandInfo = DataManager::GetInstance().GetAttackTable().GetInfo(*m_info.witchHand);
	m_witchHandFrame = 0;
	//足技とは別の番号にして、足技が当たった相手にも当たるようにする
	m_witchHandAttackId = m_player.IssueAttackId();

	//見た目はエフェクトで出す
	const Vector3& forward = m_player.GetForward();
	//プレイヤーの右方向(左手系なので前(0,0,1)の右は(1,0,0))
	const Vector3 right(forward.z, 0.0f, -forward.x);
	const Vector3 up(0.0f, 1.0f, 0.0f);

	//拳の攻撃判定
	const Vector3 hitPos = m_player.GetPos() + forward * kWitchHandForwardOffset + up * kHitHeight;
	//魔法陣を出したい位置から着弾点へ向かう方向に拳を突き出す
	const Vector3 portalPos = m_player.GetPos() - forward * kWitchHandPortalBack + up * kWitchHandPortalHeight + right * kWitchHandPortalSide;
	const Vector3 punchDir = (hitPos - portalPos).Normalize();

	//拳が着弾点に来るように、原点を拳の分だけ手前にずらす
	const Vector3 pos = hitPos - punchDir * (kWitchHandFistDistance * kWitchHandScale);

	//エフェクトの向き
	Vector3 rot;
	rot.x = asinf(punchDir.y);
	rot.y = atan2f(-punchDir.x, -punchDir.z);
	EffectManager::Instns().PlayEffect(EffectType::WickedWeave, pos, rot, kWitchHandScale);
}

void PlayerStateAttack::UpdateWitchHand()
{
	if (!m_witchHandInfo) return;

	//呼び出してからのフレームで攻撃判定を出す
	if (m_witchHandInfo->hitStartFrame <= m_witchHandFrame && m_witchHandFrame < m_witchHandInfo->hitEndFrame)
	{
		AttackRequest request{AttackData(CharacterType::Player, m_witchHandInfo->type, m_witchHandInfo->damage, m_witchHandInfo->radius)};
		request.attackId = m_witchHandAttackId;
		request.forwardOffset = kWitchHandForwardOffset;
		request.height = kHitHeight;
		m_player.RequestAttack(request);
	}
	m_witchHandFrame++;
}

void PlayerStateAttack::UpdateMove()
{
	//前進するフレームの外では止まる
	if (m_frame < m_info.moveStartFrame || m_frame >= m_info.moveEndFrame)
	{
		StopMove();
		return;
	}

	//前進の何割まで進んだかを、今のフレームと次のフレームで求める
	float moveFrame = static_cast<float>(m_info.moveEndFrame - m_info.moveStartFrame);
	float t0 = (m_frame - m_info.moveStartFrame) / moveFrame;
	float t1 = (m_frame + 1 - m_info.moveStartFrame) / moveFrame;
	//その差がこのフレームで進む距離
	float step = m_moveDistance * (EaseOut(t1) - EaseOut(t0));

	//落下の速度は残して、横の移動だけ入れる
	Vector3 move = m_player.GetForward() * step;
	auto vel = m_player.GetVelocity();
	m_player.SetVelocity(Vector3(move.x, vel.y, move.z));
}
