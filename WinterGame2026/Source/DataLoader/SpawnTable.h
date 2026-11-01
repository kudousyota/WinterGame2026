#pragma once
#include <string>
#include <vector>
#include "Vector3.h"
class SpawnTable
{
public:
	struct SpawnInfo
	{
		//ウェーブ数
		int wave = 0;
		//キャラクターの名前
		std::string characterName;
		//スポナー位置
		Vector3 spawnerPos = Vector3(0.0f, 0.0f, 0.0f);
		//スポーン位置
		Vector3 offset = Vector3(0.0f, 0.0f, 0.0f);
		//スポーンまでのフレーム数
		int delayFrame = 0;
		//実際のスポーン位置を取得する
		Vector3 GetSpawnPos() const{return spawnerPos + offset;}
	};

	//CSVの行データを受け取る
	void Load(const std::vector<std::vector<std::string>>& rows);

	//そのウェーブで出る敵の一覧
	const std::vector<SpawnInfo>& GetWave(int wave)const;
	//ウェーブ数を取得する
	int GetWaveCount()const { return static_cast<int>(m_waves.size()); }

private:
	//ウェーブ番号そのウェーブの何体目か
	std::vector<std::vector<SpawnInfo>> m_waves;
};

