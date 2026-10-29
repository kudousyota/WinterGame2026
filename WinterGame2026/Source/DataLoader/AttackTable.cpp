#include "AttackTable.h"
#include <cassert>
namespace
{
	//各CSVの列の数(足りない行は読まない)
	constexpr int kHitColumnNum = 10;
	constexpr int kMotionColumnNum = 10;
	constexpr int kComboColumnNum = 5;

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
			{ "KickDown",	AttackType::KickDown },
			{ "LegSweep",	AttackType::LegSweep },
			{ "MeleeKick",	AttackType::MeleeKick },
			{ "AirPunch",	AttackType::AirPunch },
			{ "AirPunching",	AttackType::AirPunching },
			{ "AirRush",	AttackType::AirRush },
			{ "AirSkyKick",	AttackType::AirSkyKick },
			{ "Attack",		AttackType::Attack },
			{ "Breath",		AttackType::Breath },
			{ "WitchHand",	AttackType::WitchHand },
			{ "WitchLeg",	AttackType::WitchLeg },
			{ "WitchStomp",	AttackType::WitchStomp },
		};
		auto it = kNameTable.find(name);
		assert(it != kNameTable.end() && "知らない攻撃の名前がCSVに書かれている");
		return it->second;
	}

	//Noneなら空のまま、それ以外なら攻撃の種類にする
	std::optional<AttackType> NameToOptionalAttackType(const std::string& name)
	{
		if (name == "None")
		{
			return std::nullopt;
		}
		return NameToAttackType(name);
	}
}

void AttackTable::Load(const Rows& hitRows, const Rows& motionRows, const Rows& comboRows)
{
	//先にクリアしとく
	m_infoList.clear();

	//攻撃判定(全部の攻撃にあるので、ここで攻撃を登録する)
	for (const auto& row : hitRows)
	{
		if (row.size() < kHitColumnNum)
		{
			//その行だけスキップ
			continue;
		}
		const AttackType type = NameToAttackType(row[0]);
		AttackInfo& info = m_infoList[type];
		info.type = type;
		info.hitStartFrame = std::stoi(row[1]);
		info.hitEndFrame = std::stoi(row[2]);
		info.damage = std::stoi(row[3]);
		info.radius = std::stof(row[4]);
		info.hitInterval = std::stoi(row[5]);
		info.knockback = std::stof(row[6]);
		info.canKnockDown = std::stoi(row[7]) != 0;
		info.length = std::stof(row[8]);
		info.carry = std::stoi(row[9]) != 0;
	}

	//動き(AttackHitにない名前ならat()が例外を投げるので書き忘れに気づける)
	for (const auto& row : motionRows)
	{
		if (row.size() < kMotionColumnNum)
		{
			continue;
		}
		AttackInfo& info = m_infoList.at(NameToAttackType(row[0]));
		info.totalFrame = std::stoi(row[1]);
		info.comboStartFrame = std::stoi(row[2]);
		info.cancelFrame = std::stoi(row[3]);
		info.animName = row[4];
		info.moveStartFrame = std::stoi(row[5]);
		info.moveEndFrame = std::stoi(row[6]);
		info.moveDistance = std::stof(row[7]);
		info.fallSpeed = std::stof(row[8]);
		info.riseSpeed = std::stof(row[9]);
	}

	//コンボのつながり
	for (const auto& row : comboRows)
	{
		if (row.size() < kComboColumnNum)
		{
			continue;
		}
		AttackInfo& info = m_infoList.at(NameToAttackType(row[0]));
		//Noneならそのボタンではコンボがつながらない
		info.nextPunch = NameToOptionalAttackType(row[1]);
		info.nextKick = NameToOptionalAttackType(row[2]);
		//Noneならコンボの締めでウィケッドウィーブを出さない
		info.witchHand = NameToOptionalAttackType(row[3]);
		//終わったら自動で出す攻撃
		info.autoNext = NameToOptionalAttackType(row[4]);
	}
}

const AttackTable::AttackInfo& AttackTable::GetInfo(AttackType type) const
{
	auto list = m_infoList.find(type);
	assert(list != m_infoList.end() && "攻撃データが見つからない");
	return list->second;
}
