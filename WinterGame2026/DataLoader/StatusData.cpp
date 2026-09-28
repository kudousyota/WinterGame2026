#include "StatusData.h"
#include <cassert>
namespace
{
	constexpr int kstatusSize = 5;
}
void StatusData::Load(const std::vector<std::vector<std::string>>& rows)
{
	//先にクリアしとく
	m_infoList.clear();
	for (const auto& row : rows)
	{
		if (row.size() < kstatusSize)
		{
			//その行だけスキップ
			continue;
		}

		StatusInfo info;
		info.characterName = row[0];
		info.attackPower = std::stoi(row[1]);
		//減ってくから要らない
		//info.hp	= std::stoi(row[2]);
		info.maxhp = std::stoi(row[3]);
		info.speed = std::stof(row[4]);

		m_infoList[info.characterName] = info;
	}
}

const StatusData::StatusInfo& StatusData::GetStatus(const std::string& name) const
{
	auto list = m_infoList.find(name);
	assert(list != m_infoList.end() && "ステータスが見つからない");
	return list->second;
}
