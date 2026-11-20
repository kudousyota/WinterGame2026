#pragma once
#include <vector>
#include <string>
#include "StatusData.h"
#include "AttackTable.h"
#include "SpawnTable.h"
#include "PlayerParam.h"

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
	//ゲーム中にデータを読み直す
	//読み込みに失敗したら今のデータのままにしてfalseを返す
	bool ReloadData();

	//敵のスポーンデータを返す
	const SpawnTable& GetSpawnTable() const { return m_spawnTable; }

	//キャラクターのステータスデータを返す
	const StatusData& GetStatusData() const { return m_statusData; }

	//攻撃データを返す
	const AttackTable& GetAttackTable() const { return m_attackTable; }

	//プレイヤーの手触りに関わる数値を返す
	const PlayerParam& GetPlayerParam() const { return m_playerParam; }

private:
	//コンストラクタを非公開にしてシングルトンを実現
	DataManager() = default;
	~DataManager() = default;
	DataManager(const DataManager&) = delete;
	DataManager& operator=(const DataManager&) = delete;

	//共通のCSV読み込み処理
	std::vector<std::vector<std::string>> LoadCsv(const std::string& path);

	//敵の出現データをロードする
	void LoadSpawnData();

	//キャラクターのステータスデータをロードする
	void LoadStatusData();

	//攻撃データをロードする
	void LoadAttackTable();

	//プレイヤーの数値をロードする
	void LoadPlayerParam();

	//敵の出現データ
	SpawnTable m_spawnTable;

	//キャラクターのステータスデータ
	StatusData m_statusData;

	//攻撃データ
	AttackTable m_attackTable;

	//プレイヤーの数値
	PlayerParam m_playerParam;
};