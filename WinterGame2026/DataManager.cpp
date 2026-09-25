#include "DataManager.h"

void DataManager::LoadData()
{
	//LoadSpawnData();
	LoadStatusData();
}

//void DataManager::LoadSpawnData()
//{
//	m_spawnData.Load("Data/Csv/Spawn.csv");
//}

void DataManager::LoadStatusData()
{
	CsvTable table;
	table.Load("Data/Csv/State.csv");
	m_statusData.Load(table);
}