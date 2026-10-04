#include "StatusData.h"
#include <cassert>
namespace
{
	constexpr int kstatusSize = 4;
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
		//減ってくから要らない
		//info.hp	= std::stoi(row[1]);
		info.maxhp = std::stoi(row[2]);
		info.speed = std::stof(row[3]);

		m_infoList[info.characterName] = info;
	}
}

const StatusData::StatusInfo& StatusData::GetStatus(const std::string& name) const
{
	auto list = m_infoList.find(name);
	assert(list != m_infoList.end() && "ステータスが見つからない");
	return list->second;
}
