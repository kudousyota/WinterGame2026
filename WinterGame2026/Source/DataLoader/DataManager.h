#pragma once
#include <vector>
#include <string>
#include "StatusData.h"
#include "AttackTable.h"

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
	//ゲーム中にデータを読み直す(デバッグ用)
	//読み込みに失敗したら今のデータのままにしてfalseを返す
	bool ReloadData();

	//敵のスポーンデータを返す
	//const std::vector<std::vector<std::string>>& GetSpawnData() const { return m_spawnData; }

	//キャラクターのステータスデータを返す
	const StatusData& GetStatusData() const { return m_statusData; }

	//攻撃データを返す
	const AttackTable& GetAttackTable() const { return m_attackTable; }

private:
	//コンストラクタを非公開にしてシングルトンを実現
	DataManager() = default;
	~DataManager() = default;
	DataManager(const DataManager&) = delete;
	DataManager& operator=(const DataManager&) = delete;

	//共通のCSV読み込み処理(行ごとに文字列配列にする)
	std::vector<std::vector<std::string>> LoadCsv(const std::string& path);

	//敵の出現データをロードする
	//void LoadSpawnData();

	//キャラクターのステータスデータをロードする
	void LoadStatusData();

	//攻撃データをロードする
	void LoadAttackTable();

	//敵の出現データを格納する二次元配列
	//std::vector<std::vector<std::string>> m_spawnData;

	//キャラクターのステータスデータ
	StatusData m_statusData;

	//攻撃データ
	AttackTable m_attackTable;
};