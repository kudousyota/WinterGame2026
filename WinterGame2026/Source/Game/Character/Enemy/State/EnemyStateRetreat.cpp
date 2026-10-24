#include "EnemyStateRetreat.h"
#include "EnemyStateObserve.h"

namespace
{
	//離れる速さ(移動速度に掛ける倍率)
	constexpr float kRetreatSpeedScale = 1.2f;
	//これ以上たったら離れきれなくても様子見に移る(壁際で止まり続けないようにする)
	constexpr float kMaxRetreatFrame = 90.0f;
}

void EnemyStateRetreat::Enter()
{
	m_enemy.GetAnim().ChangeAnim(m_enemy.GetMoveAnimName(), true);
}

void EnemyStateRetreat::Update()
{
	m_frame += m_enemy.GetTimeScale();

	//保つ距離まで離れたか、時間切れなら様子見へ
	if (m_enemy.GetTargetDistance() >= m_enemy.GetKeepDistance() || m_frame >= kMaxRetreatFrame)
	{
		ChangeState<EnemyStateObserve>();
		return;
	}

	//体はプレイヤーに向けたまま下がる
	m_enemy.FaceTarget();
	m_enemy.MoveAwayFromTarget(kRetreatSpeedScale);
}

void EnemyStateRetreat::Exit()
{
	m_enemy.StopMove();
}