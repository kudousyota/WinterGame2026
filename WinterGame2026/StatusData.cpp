#include "StatusData.h"
#include <cassert>

void StatusData::Load(const CsvTable& table)
{
	//先にクリアしとく
	m_infoList.clear();
	for (int row = 0; row < table.GetRowNum(); row++)
	{
		//列名で読むので、CSVの列の順番が変わっても列が増えても大丈夫
		StatusInfo info;
		info.characterName = table.GetString(row, "name");
		info.attackPower = table.GetInt(row, "attackPower");
		//hpは減っていくので読まない(maxhpで初期化する)
		info.maxhp = table.GetInt(row, "maxhp");
		info.speed = table.GetFloat(row, "speed");

		m_infoList[info.characterName] = info;
	}
}

const StatusData::StatusInfo& StatusData::GetStatus(const std::string& name) const
{
	auto list = m_infoList.find(name);
	assert(list != m_infoList.end() && "ステータスが見つからない");
	return list->second;
}
