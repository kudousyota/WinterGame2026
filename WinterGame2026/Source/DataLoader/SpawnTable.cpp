#include "SpawnTable.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdexcept>

namespace
{
	constexpr int kSpawnColumnNum = 8;
	constexpr int kWaveColumnNum = 4;
	constexpr int kGateColumnNum = 5;

	SpawnTable::SpawnBase ParseBase(const std::string& text)
	{
		if (text == "world") return SpawnTable::SpawnBase::World;
		if (text == "player") return SpawnTable::SpawnBase::Player;
		throw std::runtime_error("Spawn.csvのbaseはworldかplayer");
	}
}

bool SpawnTable::WaveArea::IsInside(const Vector3& pos) const
{
	float dx = pos.x - center.x;
	float dz = pos.z - center.z;
	return dx * dx + dz * dz <= radius * radius;
}

Vector3 SpawnTable::WaveArea::ClampInside(const Vector3& pos, float margin) const
{
	float dx = pos.x - center.x;
	float dz = pos.z - center.z;
	float dist = sqrtf(dx * dx + dz * dz);
	float maxDist = (std::max)(radius - margin, 0.0f);
	if (dist <= maxDist || dist <= 0.0f) return pos;

	//中心からの向きはそのままで、距離だけ縮める
	float scale = maxDist / dist;
	return Vector3(center.x + dx * scale, pos.y, center.z + dz * scale);
}

bool SpawnTable::WaveArea::IsSame(const WaveArea& other) const
{
	return center.x == other.center.x && center.z == other.center.z && radius == other.radius;
}

void SpawnTable::Load(const std::vector<std::vector<std::string>>& spawnRows,
                      const std::vector<std::vector<std::string>>& waveRows,
                      const std::vector<std::vector<std::string>>& gateRows)
{
	m_waves.clear();
	m_areas.clear();
	for (const auto& row : spawnRows)
	{
		//列が足りない行は書き間違いなので、読み込み失敗にする
		//(スキップすると敵が1体足りないことに気づきにくい)
		if (row.size() < kSpawnColumnNum)
		{
			throw std::runtime_error("Spawn.csvの列が足りない");
		}

		SpawnInfo info;
		info.wave = std::stoi(row[0]);
		info.characterName = row[1];
		info.base = ParseBase(row[2]);
		//高さは地面から決めるので、xとzだけ読む
		info.spawnerPos = Vector3(std::stof(row[3]), 0.0f, std::stof(row[4]));
		info.offset = Vector3(std::stof(row[5]), 0.0f, std::stof(row[6]));
		info.delayFrame = std::stoi(row[7]);

		//まだないウェーブ番号なら、そこまで配列を広げる
		if (info.wave >= static_cast<int>(m_waves.size()))
		{
			m_waves.resize(info.wave + 1);
		}
		m_waves[info.wave].push_back(info);
	}

	//Wave.csvに書いていないウェーブはエリアなしにする
	m_areas.resize(m_waves.size());
	for (const auto& row : waveRows)
	{
		if (row.size() < kWaveColumnNum)
		{
			throw std::runtime_error("Wave.csvの列が足りない");
		}
		int wave = std::stoi(row[0]);
		//敵がいないウェーブのエリアは使わない
		if (wave < 0 || wave >= static_cast<int>(m_areas.size())) continue;

		WaveArea& area = m_areas[wave];
		area.center = Vector3(std::stof(row[1]), 0.0f, std::stof(row[2]));
		area.radius = std::stof(row[3]);
	}

	//ゲートは1つのウェーブに何個でも書けるので、行ごとに足していく
	for (const auto& row : gateRows)
	{
		if (row.size() < kGateColumnNum)
		{
			throw std::runtime_error("Gate.csvの列が足りない");
		}
		int wave = std::stoi(row[0]);
		//敵がいないウェーブは始まらないので、ゲートも使わない
		if (wave < 0 || wave >= static_cast<int>(m_areas.size())) continue;

		GateInfo gate;
		gate.pos = Vector3(std::stof(row[1]), std::stof(row[2]), std::stof(row[3]));
		//CSVは度で書いた方が読みやすいので、ここでラジアンに直す
		gate.rotY = std::stof(row[4]) * (DX_PI_F / 180.0f);
		m_areas[wave].gates.push_back(gate);
	}
}

int SpawnTable::GetMaxGateNum() const
{
	size_t maxNum = 0;
	for (const auto& area : m_areas)
	{
		maxNum = (std::max)(maxNum, area.gates.size());
	}
	return static_cast<int>(maxNum);
}

const std::vector<SpawnTable::SpawnInfo>& SpawnTable::GetWave(int wave) const
{
	assert(0 <= wave && wave < GetWaveCount() && "ウェーブ番号が範囲外");
	return m_waves[wave];
}

const SpawnTable::WaveArea& SpawnTable::GetArea(int wave) const
{
	assert(0 <= wave && wave < GetWaveCount() && "ウェーブ番号が範囲外");
	return m_areas[wave];
}
