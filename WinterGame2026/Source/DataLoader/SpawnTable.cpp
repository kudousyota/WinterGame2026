#include "SpawnTable.h"
#include <cassert>
#include <stdexcept>

namespace
{
	constexpr int kSpawnColumnNum = 7;
}

void SpawnTable::Load(const std::vector<std::vector<std::string>>& rows)
{
	m_waves.clear();
	for (const auto& row : rows)
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
		//高さはSpawnEnemyで足すので、xとzだけ読む
		info.spawnerPos = Vector3(std::stof(row[2]), 0.0f, std::stof(row[3]));
		info.offset = Vector3(std::stof(row[4]), 0.0f, std::stof(row[5]));
		info.delayFrame = std::stoi(row[6]);

		//まだないウェーブ番号なら、そこまで配列を広げる
		if (info.wave >= static_cast<int>(m_waves.size()))
		{
			m_waves.resize(info.wave + 1);
		}
		m_waves[info.wave].push_back(info);
	}
}

const std::vector<SpawnTable::SpawnInfo>& SpawnTable::GetWave(int wave) const
{
	assert(0 <= wave && wave < GetWaveCount() && "ウェーブ番号が範囲外");
	return m_waves[wave];
}

