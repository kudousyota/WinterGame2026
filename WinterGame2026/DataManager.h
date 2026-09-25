#pragma once
#include <vector>
#include <string>
#include "StatusData.h"

class DataManager
{
public:
	static DataManager& GetInstance()
	{
		static DataManager instance;
		return instance;
	}

	//データの読み込み
	void LoadData();

	//敵のスポーンデータを返す
	//const CsvTable& GetSpawnData() const { return m_spawnData; }

	//キャラクターのステータスデータを返す
	const StatusData& GetStatusData() const { return m_statusData; }

private:
	//コンストラクタを非公開にしてシングルトンを実現
	DataManager() = default;
	~DataManager() = default;
	DataManager(const DataManager&) = delete;
	DataManager& operator=(const DataManager&) = delete;

	//敵の出現データをロードする
	//void LoadSpawnData();

	//キャラクターのステータスデータをロードする
	void LoadStatusData();

	//敵の出現データ
	//CsvTable m_spawnData;

	//キャラクターのステータスデータ
	StatusData m_statusData;
};