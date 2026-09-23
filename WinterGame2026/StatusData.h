#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "Vector3.h"
class StatusData
{
public:
	enum class CharacterType
	{
		Player,
		Enemy,
	};
	struct StatusInfo
	{
		//キャラクターの名前
		std::string characterName;
		//攻撃力
		int attackPower = 0;
		//体力
		//int hp = 0;
		//最大体力
		int maxhp = 0;
		//速度
		float speed = 0;
	};

	//CSVの行データを受け取る
	void Load(const std::vector<std::vector<std::string>>& rows);

	//名前からステータスを取得
	const StatusInfo& GetStatus(const std::string& name)const;
private:
	std::unordered_map<std::string, StatusInfo> m_infoList;
};

