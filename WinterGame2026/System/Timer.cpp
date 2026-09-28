#include "Timer.h"
#include <algorithm>
//シングルトンのTimerを取得
Timer& Timer::Instance()
{
	static Timer timer;
	return timer;
}

void Timer::Update()
{
	//ゲーム全体の時間
	if (m_frameCount > 0)
	{
		m_frameCount--;
		if (m_frameCount <= 0)
		{
			m_timeScale = 1.0f;
		}
	}
	if (m_frameCountEnemy > 0)
	{
		m_frameCountEnemy--;
		if (m_frameCountEnemy <= 0)
		{
			m_enemyTimeScale = 1.0f;
		}
	}
}

void Timer::SetTimeScaleForFrames(float timescale, int frames)
{
	//時間のスケールを設定
	m_timeScale = timescale;
	//フレームカウントを設定する
	m_frameCount = frames;
}

void Timer::SetEnemyTimeScaleForFrames(float scale, int frames)
{
	//敵の時間のスケールを設定する
	m_enemyTimeScale = scale;
	//敵のフレームカウントを設定する
	m_frameCountEnemy = frames;
}

bool Timer::IsEnemySlow() const
{
	return m_frameCountEnemy > 0;
}
