#include "AttackTable.h"
#include <cassert>
namespace
{
	constexpr int kAttackInfoSize = 17;
	//後から足した列の場所(書いていない行もある)
	constexpr int kKnockbackColumn = 17;
	constexpr int kKnockdownColumn = 18;
	constexpr int kLengthColumn = 19;

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
			{ "AirPunch",	AttackType::AirPunch },
			{ "AirPunching",	AttackType::AirPunching },
			{ "AirRush",	AttackType::AirRush },
			{ "AirSkyKick",	AttackType::AirSkyKick },
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
		info.cancelFrame = std::stoi(row[5]);
		info.damage = std::stoi(row[6]);
		info.radius = std::stof(row[7]);
		info.moveStartFrame = std::stoi(row[8]);
		info.moveEndFrame = std::stoi(row[9]);
		info.moveDistance = std::stof(row[10]);
		info.animName = row[11];
		//Noneならそのボタンではコンボがつながらない
		if (row[12] != "None")
		{
			info.nextPunch = NameToAttackType(row[12]);
		}
		if (row[13] != "None")
		{
			info.nextKick = NameToAttackType(row[13]);
		}
		//Noneならコンボの締めでウィケッドウィーブを出さない
		if (row[14] != "None")
		{
			info.witchHand = NameToAttackType(row[14]);
		}
		info.hitInterval = std::stoi(row[15]);

		info.fallSpeed = std::stof(row[16]);

		// 後から足した列は、書いていない行もあるので空なら初期値のままにする
		auto hasColumn = [&row](int index)
		{
			return row.size() > static_cast<size_t>(index) && !row[index].empty();
		};
		//吹っ飛ぶ距離
		if (hasColumn(kKnockbackColumn))
		{
			info.knockback = std::stof(row[kKnockbackColumn]);
		}
		//叩き落とせるか(1なら叩き落とす)
		if (hasColumn(kKnockdownColumn))
		{
			info.canKnockDown = std::stoi(row[kKnockdownColumn]) != 0;
		}
		//攻撃判定の長さ
		if (hasColumn(kLengthColumn))
		{
			info.length = std::stof(row[kLengthColumn]);
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
