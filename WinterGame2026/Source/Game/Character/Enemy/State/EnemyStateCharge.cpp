#include "EnemyStateCharge.h"
#include "EnemyStateRetreat.h"
#include "../../../../DataLoader/DataManager.h"
#include "../../../../Effect/EffectManager.h"
#include <cstdio>
#include <cmath>


EnemyStateCharge::EnemyStateCharge(Enemy& enemy) : 
	EnemyStateBase(enemy),
	m_info(DataManager::GetInstance().GetAttackTable().GetInfo(enemy.GetAttackType()))
{
	//進む距離を1フレームの速さに直す
	const int moveFrame = m_info.moveEndFrame - m_info.moveStartFrame;
	if (moveFrame > 0)
	{
		m_chargeSpeed = m_info.moveDistance / static_cast<float>(moveFrame);
	}
}

void EnemyStateCharge::Enter()
{
	m_enemy.StopMove();
	m_enemy.FaceTarget();

	//溜めがある攻撃は、最初はゆっくり再生する
	const float animScale = (m_info.windupFrame > 0) ? m_info.windupAnimScale : 1.0f;
	m_enemy.GetAnim().ChangeAnim(m_info.animName, false, animScale);
	m_attackId = m_enemy.IssueAttackId();

	//突進が始まるまでが予兆
	m_enemy.SetWarning(true);

	//予兆エフェクトを出す
	const Vector3& forward = m_enemy.GetForward();
	const Vector3 effectPos = m_enemy.GetPos() + forward * m_enemy.GetHitForwardOffset() + Vector3(0.0f, m_enemy.GetHitHeight(), 0.0f);
	Vector3 rot;
	rot.y = atan2f(-forward.x, -forward.z);
	m_telegraphHandle = EffectManager::Instns().PlayEffect(EffectType::AttackTelegraph, effectPos, rot);
}

void EnemyStateCharge::Update()
{
	//攻撃が終わったらプレイヤーから離れる
	if (m_frame >= m_info.totalFrame)
	{
		ChangeState<EnemyStateRetreat>();
		return;
	}

	//溜めが終わったら普通の速度に戻す
	if (m_info.windupFrame > 0 && m_frame >= m_info.windupFrame)
	{
		m_enemy.GetAnim().SetAnimTimeScale(1.0f);
	}

	if (m_frame < m_info.moveStartFrame)
	{
		//溜めている間はプレイヤーの方を向き続ける
		m_enemy.StopMove();
		m_enemy.FaceTarget();
	}
	else if (IsCharging())
	{
		//突進を始めた瞬間に方向を決める
		if (!m_hasStarted)
		{
			StartCharge();
		}
		//決めた方向へ真っ直ぐすすむ
		m_enemy.SetVelocity(m_chargeDir * m_chargeSpeed);
	}
	else
	{
		//走り終わり//ここで背中を見せるようにする
		m_enemy.StopMove();
	}
	//判定が出るまでが予兆
	m_enemy.SetWarning(m_frame < m_info.hitStartFrame);

	if (IsHitActive())
	{
		AttackRequest request{ AttackData(CharacterType::Enemy,m_info.type,m_info.damage,m_info.radius) };
		request.attackId = m_attackId;
		request.forwardOffset = m_enemy.GetHitForwardOffset();
		request.height = m_enemy.GetHitHeight();
		request.dir = m_chargeDir;
		m_enemy.RequestAttack(request);
	}
	m_frame += m_enemy.GetTimeScale();
}

void EnemyStateCharge::Exit()
{
	//途中で別のステートになっても走り続けないようにする
	m_enemy.StopMove();
	m_enemy.SetWarning(false);
	EffectManager::Instns().StopEffect(m_telegraphHandle);
	//攻撃が終わったので他の敵が攻撃ができるようにする
	m_enemy.ReleaseAttackToken();
}

bool EnemyStateCharge::IsHitActive() const
{
	return m_info.hitStartFrame <= m_frame && m_frame < m_info.hitEndFrame;
}

bool EnemyStateCharge::IsCharging() const
{
	return m_info.moveStartFrame <= m_frame && m_frame < m_info.moveEndFrame;
}

void EnemyStateCharge::StartCharge()
{
	m_hasStarted = true;
	//今向いている方向に突進する
	Vector3 dir = m_enemy.GetForward();
	dir.y = 0.0f;
	if (dir.SqMagnitude() <= 0.0f)dir = Vector3(0.0f, 0.0f, 1.0f);
	m_chargeDir = dir.Normalize();

	//突進が始まったら予兆エフェクトは消す
	EffectManager::Instns().StopEffect(m_telegraphHandle);
	//走りアニメーションに切り替える
	m_enemy.GetAnim().ChangeAnim(m_enemy.GetChargeRunAnimName(), true);
}

#ifdef _DEBUG
std::string EnemyStateCharge::GetDebugInfo() const
{
	//今どの段階か
	const char* phase = "Windup";
	if (m_frame >= m_info.moveEndFrame) phase = "Recover";
	else if (m_frame >= m_info.moveStartFrame) phase = "Charge";

	char buf[160];
	sprintf_s(buf, "%s  Frame:%.1f/%d  Phase:%s  Hit:%s  Id:%d",
		m_info.animName.c_str(), m_frame, m_info.totalFrame, phase, IsHitActive() ? "ON" : "OFF", m_attackId);
	return buf;
}

#endif