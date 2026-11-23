#include "EnemyStateStun.h"
#include "EnemyStateRetreat.h"
#include <algorithm>
#include <cstdio>

namespace
{
	//壁から跳ね返る距離
	constexpr float kBounceDistance = 150.0f;
	//跳ね返りにかけるフレーム数
	constexpr float kBounceFrame = 15.0f;
	//ふらついている全体のフレーム数//アニメーションより短くして途中で終わらないようにする
	constexpr float kStunFrame = 165.0f;

	//跳ね返った瞬間が一番速く、だんだんゆっくりになる
	float EaseOut(float t)
	{
		return 1.0f - (1.0f - t) * (1.0f - t);
	}
}

EnemyStateStun::EnemyStateStun(Enemy& enemy, const Vector3& bounceDir) :
	EnemyStateBase(enemy)
{
	m_bounceDir = bounceDir;
}

void EnemyStateStun::Enter()
{
	m_enemy.StopMove();
	//頭を抱えてふらつく//ループさせると最後で姿勢が飛ぶので1回だけ流す
	m_enemy.GetAnim().ChangeAnim(m_enemy.GetStunAnimName(), false);
}

void EnemyStateStun::Update()
{
	m_enemy.StopMove();

	//ふらつき終わったらプレイヤーから離れる
	if (m_frame >= kStunFrame)
	{
		ChangeState<EnemyStateRetreat>();
		return;
	}

	m_frame += m_enemy.GetTimeScale();

	//最初の少しの間だけ壁から跳ね返る
	//向きは変えないので、壁を向いたまま背中の弱点がプレイヤー側に出る
	const float t = std::clamp(m_frame / kBounceFrame, 0.0f, 1.0f);
	const float move = (EaseOut(t) - EaseOut(m_prevT)) * kBounceDistance;
	m_enemy.SetPosition(m_enemy.GetPos() + m_bounceDir * move);
	m_prevT = t;
}

#ifdef _DEBUG
std::string EnemyStateStun::GetDebugInfo() const
{
	char buf[64];
	sprintf_s(buf, "Frame:%.1f/%.0f", m_frame, kStunFrame);
	return buf;
}
#endif
