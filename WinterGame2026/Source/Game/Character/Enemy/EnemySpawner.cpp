#include "EnemySpawner.h"
#include <algorithm>

namespace
{
	//ウェーブを全滅させてから、次のウェーブを始めるまでのフレーム数
	constexpr int kWaveIntervalFrame = 90;
	//敵がエリアの壁に重なって出ないよう、壁から離す距離
	constexpr float kSpawnWallMargin = 150.0f;
}

void EnemySpawner::Init(const SpawnTable& table)
{
	m_table = table;
	m_requests.clear();
	m_waitFrame = 0;
	StartWave(0);
}

void EnemySpawner::Update(int aliveCount, const SpawnContext& context)
{
	switch (m_state)
	{
	case State::Finished:
		return;

	case State::Interval:
		//次のウェーブを始めるまで待つ
		m_waitFrame--;
		if (m_waitFrame <= 0) StartWave(m_waveIndex);
		return;

	case State::WaitEnter:
		//エリアがないウェーブはすぐ始める
		if (m_area.HasArea() && !m_area.IsInside(context.playerPos)) return;
		BeginBattle(context);
		break;

	case State::Battle:
		break;
	}

	//全部出し終わって生きている敵がいなければ次のウェーブに進む
	if (IsWaveAllSpawned() && aliveCount == 0)
	{
		m_waveIndex++;
		//最後のウェーブだったらここで終わり
		if (IsAllWavesFinished())
		{
			m_state = State::Finished;
			return;
		}
		m_waitFrame = kWaveIntervalFrame;
		m_state = State::Interval;
		return;
	}

	//出す時間になった敵をまとめて出してもらう
	while (m_nextIndex < m_currentWave.size() && m_currentWave[m_nextIndex].delayFrame <= m_waveFrame)
	{
		const auto& info = m_currentWave[m_nextIndex];
		m_requests.push_back({ info.characterName, ToWorldPos(info) });
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

bool EnemySpawner::IsBattleLocked() const
{
	if (!m_area.HasArea()) return false;
	if (m_state == State::Battle) return true;
	//同じエリアで次のウェーブが続くなら、合間も出られないままにする
	//(合間に抜け出すと、次のウェーブが始まらなくなる)
	if (m_state == State::Interval)
	{
		return m_table.GetArea(m_waveIndex).IsSame(m_area);
	}
	return false;
}

void EnemySpawner::StartWave(int waveIndex)
{
	m_waveIndex = waveIndex;
	m_nextIndex = 0;
	m_waveFrame = 0;
	m_currentWave.clear();
	if (IsAllWavesFinished())
	{
		m_state = State::Finished;
		return;
	}

	m_currentWave = m_table.GetWave(waveIndex);
	m_area = m_table.GetArea(waveIndex);
	//CSVの行の順番がバラバラでも、出す順に並ぶようにする
	std::stable_sort(m_currentWave.begin(), m_currentWave.end(), [](const auto& a, const auto& b) { return a.delayFrame < b.delayFrame; });
	m_state = State::WaitEnter;
}

void EnemySpawner::BeginBattle(const SpawnContext& context)
{
	m_basePos = context.playerPos;
	m_baseForward = context.forward;
	m_baseRight = context.right;
	m_waveFrame = 0;
	m_state = State::Battle;
}

bool EnemySpawner::IsWaveAllSpawned() const
{
	return m_nextIndex >= m_currentWave.size();
}

Vector3 EnemySpawner::ToWorldPos(const SpawnTable::SpawnInfo& info) const
{
	Vector3 local = info.GetLocalPos();
	Vector3 pos = local;
	if (info.base == SpawnTable::SpawnBase::Player)
	{
		//xはカメラの右、zはカメラの正面に向ける(画面の奥に出したいときはzを大きくする)
		pos = m_basePos + m_baseRight * local.x + m_baseForward * local.z;
	}
	//高さは地面から決める(プレイヤーの高さを目安にする)
	pos.y = m_basePos.y;

	//エリアの外や壁の向こうに出ないよう、内側に収める
	if (m_area.HasArea())
	{
		pos = m_area.ClampInside(pos, kSpawnWallMargin);
	}
	return pos;
}
