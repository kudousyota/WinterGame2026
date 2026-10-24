#include "EnemyStateObserve.h"
#include "EnemyStateIdle.h"
#include "EnemyStateRetreat.h"
#include "DxLib.h"

namespace
{
	//様子見を続けるフレーム数
	constexpr float kObserveFrame = 90.0f;
	//横に回る速さ(移動速度に掛ける倍率)
	constexpr float kStrafeSpeedScale = 0.5f;
}

void EnemyStateObserve::Enter()
{
	m_enemy.GetAnim().ChangeAnim(m_enemy.GetMoveAnimName(), true);
	//右回りか左回りかはランダムに決める(GetRand(1)は0か1を返す)
	m_side = (GetRand(1) == 0) ? 1.0f : -1.0f;
}

void EnemyStateObserve::Update()
{
	m_frame += m_enemy.GetTimeScale();

	//近づかれすぎたらもう一度離れる
	if (m_enemy.GetTargetDistance() < m_enemy.GetTooCloseDistance())
	{
		ChangeState<EnemyStateRetreat>();
		return;
	}
	//しばらく様子を見たら次の行動を決める
	if (m_frame >= kObserveFrame)
	{
		ChangeState<EnemyStateIdle>();
		return;
	}

	m_enemy.FaceTarget();
	m_enemy.StrafeAroundTarget(kStrafeSpeedScale, m_side);
}

void EnemyStateObserve::Exit()
{
	m_enemy.StopMove();
}