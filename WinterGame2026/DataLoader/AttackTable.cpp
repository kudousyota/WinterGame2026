#include "AttackTable.h"
#include <cassert>
namespace
{
	constexpr int kAttackInfoSize = 9;

	//CSVに書いた名前をAttackTypeに変換する
	AttackType NameToAttackType(const std::string& name)
	{
		static const std::unordered_map<std::string, AttackType> kNameTable =
		{
			{ "Punch",		AttackType::Punch },
			{ "Rush",		AttackType::Rush },
			{ "Kick",		AttackType::Kick },
			{ "JumpKick",	AttackType::JumpKick },
			{ "Punching",	AttackType::Punching },
			{ "Kicking",	AttackType::Kicking },
			{ "MmaKick",	AttackType::MmaKick },
			{ "SideKick",	AttackType::SideKick },
			{ "RotateKick",	AttackType::RotateKick },
			{ "HurricaneKick",	AttackType::HurricaneKick },
			{ "LegSweep",	AttackType::LegSweep },
			{ "MeleeKick",	AttackType::MeleeKick },
			{ "Attack",		AttackType::Attack },
			{ "Breath",		AttackType::Breath },
			{ "WitchHand",	AttackType::WitchHand },
		};
		auto it = kNameTable.find(name);
		assert(it != kNameTable.end() && "知らない攻撃の名前がCSVに書かれている");
		return it->second;
	}
}

void AttackTable::Load(const std::vector<std::vector<std::string>>& rows)
{
	//先にクリアしとく
	m_infoList.clear();
	for (const auto& row : rows)
	{
		if (row.size() < kAttackInfoSize)
		{
			//その行だけスキップ
			continue;
		}

		AttackInfo info;
		info.type = NameToAttackType(row[0]);
		info.totalFrame = std::stoi(row[1]);
		info.hitStartFrame = std::stoi(row[2]);
		info.hitEndFrame = std::stoi(row[3]);
		info.comboStartFrame = std::stoi(row[4]);
		info.damage = std::stoi(row[5]);
		info.radius = std::stof(row[6]);
		info.animName = row[7];
		//Noneならコンボなし
		if (row[8] != "None")
		{
			info.next = NameToAttackType(row[8]);
		}

		m_infoList[info.type] = info;
	}
}

const AttackTable::AttackInfo& AttackTable::GetInfo(AttackType type) const
{
	auto list = m_infoList.find(type);
	assert(list != m_infoList.end() && "攻撃データが見つからない");
	return list->second;
}
