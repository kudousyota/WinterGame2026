#pragma once
#include <vector>
#include <string>
#include "Vector3.h"
class StatusData
{
public:
	enum class CharacterType
	{
		Player,
		Enemye,
	};
	struct StateusInfo
	{
		//キャラクターの名前
		std::string characterName;
		//攻撃力
		int attackPower;
		//体力
		int hp;
		//最大体力
		int maxhp;
		//速度
		float speed;
	};
};

