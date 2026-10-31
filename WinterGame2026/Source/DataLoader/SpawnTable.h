#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Vector3.h"
class SpawnTable
{
public:
	struct SpawnInfo
	{
		//ウェーブ数
		int weve = 0;
		//キャラクターの名前
		std::string characterName;
		//スポナー位置
		Vector3 spawnerPos = Vector3(0.0f, 0.0f, 0.0f);
		//スポーン位置
		Vector3 spawnPos = Vector3(0.0f, 0.0f, 0.0f);
		//倒さないと行けない数
		float killCount = 0.0f;
	};

	//CSVの行データを受け取る
	void Load(const std::vector<std::vector<std::string>>& rows);

private:
	std::unordered_map<std::string, SpawnInfo> m_infoList;
};

