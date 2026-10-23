#include "EnemyStateDeath.h"
#include <algorithm>
#include <cstdio>

namespace
{
	//倒れ終わってから消え始めるまでのフレーム数(倒れた姿を少し見せる)
	constexpr float kLieFrame = 30.0f;
	//消え始めてから完全に消えるまでのフレーム数
	constexpr float kFadeFrame = 60.0f;
}

void EnemyStateDeath::Enter()
{
	m_enemy.StopMove();
	m_enemy.GetAnim().ChangeAnim(m_enemy.GetDeathAnimName(), false);
}

void EnemyStateDeath::Update()
{
	m_enemy.StopMove();

	//倒れるアニメーションが終わるまで待つ
	if (!m_enemy.GetAnim().GetAnimEndFlag()) return;

	m_afterAnimFrame += m_enemy.GetTimeScale();

	//倒れたまま少し見せてから、だんだん透明にする
	float fadeRate = (m_afterAnimFrame - kLieFrame) / kFadeFrame;
	fadeRate = std::clamp(fadeRate, 0.0f, 1.0f);
	m_enemy.SetOpacity(1.0f - fadeRate);

	//完全に透明になったら消えたことにする
	if (fadeRate >= 1.0f)
	{
		m_enemy.Vanish();
	}
}

#ifdef _DEBUG
std::string EnemyStateDeath::GetDebugInfo() const
{
	char buf[64];
	sprintf_s(buf, "AfterAnim:%.1f  Opacity:%.2f", m_afterAnimFrame, m_enemy.GetOpacity());
	return buf;
}
#endif
