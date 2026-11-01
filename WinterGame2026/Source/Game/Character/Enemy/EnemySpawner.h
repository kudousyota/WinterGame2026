#pragma once
#include <string>
#include <vector>
#include "../../../DataLoader/SpawnTable.h"

//敵のスポーン要求
struct SpawnRequest
{
	std::string characterName;
	//地面の位置
	Vector3 groundPos;
};

class EnemySpawner
{
public:
	void Init(const SpawnTable& table);
	/// <summary>
	/// スポーン要求を更新する
	/// </summary>
	/// <param name="aliveCount">生存している敵の数</param>
	void Update(int aliveCount);
	//このフレームに出す敵の依頼を受け取る
	std::vector<SpawnRequest>TakeSpawnRequests();
	//最後のウェーブまで倒したか
	bool IsAllWavesFinished()const;
	//今のウェーブ番号
	int GetCurrentWave()const { return m_waveIndex; }

private:
	//ウェーブを始める
	void StartWave(int waveIndex);
	//今のウェーブの敵を全部出し終わったか
	bool IsWaveAllSpawned()const;

	SpawnTable m_table;
	//今のウェーブ番号
	int m_waveIndex = 0;
	//今のウェーブの敵
	std::vector<SpawnTable::SpawnInfo> m_currentWave;
	//次に出す敵がm_currentWaveの何番目か
	size_t m_nextIndex = 0;
	//次の敵を出すまでのフレーム数
	int m_waveFrame = 0;
	//次のウェーブを進めるまでの残りフレーム数
	int m_waitFrame = 0;

	std::vector<SpawnRequest> m_requests;
};

