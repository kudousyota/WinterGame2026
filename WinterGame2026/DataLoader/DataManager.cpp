#include "DataManager.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include <stdexcept>

void DataManager::LoadData()
{
	//LoadSpawnData();
	LoadStatusData();
	LoadAttackTable();
}

bool DataManager::ReloadData()
{
	//一旦別の変数に読み込んで、全部成功したときだけ差し替える
	//(編集途中のCSVで数字が読めなくても、今のデータが壊れないようにする)
	StatusData statusData;
	AttackTable attackTable;
	try
	{
		statusData.Load(LoadCsv("Data/Csv/State.csv"));
		attackTable.Load(LoadCsv("Data/Csv/Attack.csv"));
	}
	catch (const std::exception&)
	{
		return false;
	}
	m_statusData = std::move(statusData);
	m_attackTable = std::move(attackTable);
	return true;
}

//CSVを読んで行ごとの文字列配列にする共通処理
std::vector<std::vector<std::string>> DataManager::LoadCsv(const std::string& path)
{
	std::vector<std::vector<std::string>> result;

	std::ifstream file(path);
	assert(file.is_open() && "CSVファイルが開けない");

	std::string line;
	while (std::getline(file, line))
	{
		//Windows改行対策
		if (!line.empty() && line.back() == '\r')
		{
			line.pop_back();
		}
		if (line.empty() || line[0] == '#')
		{
			//コメント行や空行はスキップ
			continue;
		}

		std::istringstream iss(line);
		std::string token;
		std::vector<std::string> tokens;
		//カンマで分割してトークンを取得
		while (std::getline(iss, token, ','))
		{
			tokens.push_back(token);
		}
		//分割したトークンのベクターを生データとして保存
		result.push_back(tokens);
	}
	return result;
}

//void DataManager::LoadSpawnData()
//{
//	m_spawnData = LoadCsv("Data/Csv/Spawn.csv");
//}

void DataManager::LoadStatusData()
{
	m_statusData.Load(LoadCsv("Data/Csv/State.csv"));
}

void DataManager::LoadAttackTable()
{
	m_attackTable.Load(LoadCsv("Data/Csv/Attack.csv"));
}