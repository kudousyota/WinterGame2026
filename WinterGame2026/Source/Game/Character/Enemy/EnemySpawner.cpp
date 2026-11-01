#include "EnemySpawner.h"
#include <algorithm>

namespace
{
	//ウェーブを全滅させてから、次のウェーブを始めるまでのフレーム数
	constexpr int kWaveIntervalFrame = 90;
}

void EnemySpawner::Init(const SpawnTable& table)
{
	m_table = table;
	m_requests.clear();
	m_waitFrame = 0;
	StartWave(0);
}

void EnemySpawner::Update(int aliveCount)
{
	if (IsAllWavesFinished())return;
	//次のウェーブを始めるまで待つ
	if (m_waitFrame > 0)
	{
		m_waitFrame--;
		if (m_waitFrame == 0)StartWave(m_waveIndex);
		return;
	}

	//全部出し終わって生きている敵がいなければ次のウェーブに進む
	if (IsWaveAllSpawned() && aliveCount == 0)
	{
		m_waveIndex++;
		//最後のウェーブだったらここで終わり
		if (IsAllWavesFinished())return;
		m_waitFrame = kWaveIntervalFrame;
		return;
	}

	//出す時間になった敵をまとめて出してもらう
	while (m_nextIndex < m_currentWave.size() &&m_currentWave[m_nextIndex].delayFrame <= m_waveFrame)
	{
		const auto& info = m_currentWave[m_nextIndex];
		m_requests.push_back({ info.characterName,info.GetSpawnPos() });
		m_nextIndex++;
	}
	m_waveFrame++;
}

std::vector<SpawnRequest> EnemySpawner::TakeSpawnRequests()
{
	//仲間を渡してリストを空にする
	//std::exchangeは第一引数の値を返しつつ第二引数の値で第一引数を置き換える関数
	return std::exchange(m_requests, {});
}

bool EnemySpawner::IsAllWavesFinished() const
{
	return m_waveIndex >= m_table.GetWaveCount();
}

void EnemySpawner::StartWave(int waveIndex)
{
	m_waveIndex = waveIndex;
	m_nextIndex = 0;
	m_waveFrame = 0;
	m_currentWave.clear();
	if (IsAllWavesFinished()) return;

	m_currentWave = m_table.GetWave(waveIndex);
	//CSVの行の順番がバラバラでも、出す順に並ぶようにする
	std::stable_sort(m_currentWave.begin(), m_currentWave.end(),[](const auto& a, const auto& b) { return a.delayFrame < b.delayFrame; });
}

bool EnemySpawner::IsWaveAllSpawned() const
{
	return m_nextIndex >= m_currentWave.size();
}

