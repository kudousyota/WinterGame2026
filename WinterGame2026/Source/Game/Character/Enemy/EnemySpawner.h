#pragma once
#include <string>
#include <vector>
#include "../../../DataLoader/SpawnTable.h"

//敵のスポーン要求
struct SpawnRequest
{
	std::string characterName;
	//出す位置
	Vector3 groundPos;
};

//スポーン位置を決めるのに使う、今のプレイヤーとカメラの情報
struct SpawnContext
{
	Vector3 playerPos;
	//カメラの正面と右
	Vector3 forward;
	Vector3 right;
};

class EnemySpawner
{
public:
	void Init(const SpawnTable& table);
	/// <summary>
	/// スポーン要求を更新する
	/// </summary>
	/// <param name="aliveCount">生存している敵の数</param>
	/// <param name="context">プレイヤーとカメラの情報</param>
	void Update(int aliveCount, const SpawnContext& context);
	//このフレームに出す敵の依頼を受け取る
	std::vector<SpawnRequest>TakeSpawnRequests();
	//最後のウェーブまで倒したか
	bool IsAllWavesFinished()const;
	//今のウェーブ番号
	int GetCurrentWave()const { return m_waveIndex; }
	//戦闘エリアから出られない状態か
	bool IsBattleLocked()const;
	//今の戦闘エリア(IsBattleLockedのときだけ使う)
	const SpawnTable::WaveArea& GetBattleArea()const { return m_area; }

private:
	enum class State
	{
		//プレイヤーがエリアに入るのを待っている
		WaitEnter,
		//敵を出して、全滅するのを待っている
		Battle,
		//全滅させて、次のウェーブまで待っている
		Interval,
		//全部のウェーブが終わった
		Finished,
	};

	//ウェーブを準備する(プレイヤーがエリアに入るのを待つ)
	void StartWave(int waveIndex);
	//ウェーブの敵を出し始める
	void BeginBattle(const SpawnContext& context);
	//今のウェーブの敵を全部出し終わったか
	bool IsWaveAllSpawned()const;
	//CSVの位置を、ワールドの位置にする
	Vector3 ToWorldPos(const SpawnTable::SpawnInfo& info)const;

	SpawnTable m_table;
	State m_state = State::WaitEnter;
	//今のウェーブ番号
	int m_waveIndex = 0;
	//今のウェーブの敵
	std::vector<SpawnTable::SpawnInfo> m_currentWave;
	//今のウェーブの戦闘エリア
	SpawnTable::WaveArea m_area;
	//次に出す敵がm_currentWaveの何番目か
	size_t m_nextIndex = 0;
	//ウェーブが始まってから経ったフレーム数
	int m_waveFrame = 0;
	//次のウェーブを進めるまでの残りフレーム数
	int m_waitFrame = 0;

	//プレイヤー基準の敵を出すときの基準(ウェーブ開始時に決めて、隊形が崩れないようにする)
	Vector3 m_basePos;
	Vector3 m_baseForward = Vector3(0.0f, 0.0f, 1.0f);
	Vector3 m_baseRight = Vector3(1.0f, 0.0f, 0.0f);

	std::vector<SpawnRequest> m_requests;
};
