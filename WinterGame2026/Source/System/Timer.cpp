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
	//ヒットストップ
	m_isHitStop = m_hitStopFrame > 0;
	if (m_isHitStop)
	{
		m_hitStopFrame--;
		//止まっている間はウィッチタイムなどの残り時間も減らさない
		return;
	}


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
	//ゲーム内の時間を進める
	m_gameTime++;
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

void Timer::Reset()
{
	m_timeScale = 1.0f;
	m_frameCount = 0;
	m_enemyTimeScale = 1.0f;
	m_frameCountEnemy = 0;
	m_hitStopFrame = 0;
	m_isHitStop = false;
}

void Timer::RequestHitStop(int frame)
{
	//連続で当たっても止まる時間が伸び続けないように、長い方を使う
	m_hitStopFrame = (std::max)(m_hitStopFrame, frame);
}
