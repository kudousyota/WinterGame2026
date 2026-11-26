#include "PlayerParam.h"
#include <unordered_map>
#include <stdexcept>
#include <cmath>

namespace
{
	//名前と値の2列が必要
	constexpr int kParamSize = 2;
}

void PlayerParam::Load(const std::vector<std::vector<std::string>>& rows)
{
	//いったん名前と値の組にする
	std::unordered_map<std::string, float> values;
	for (const auto& row : rows)
	{
		if (row.size() < kParamSize)
		{
			//その行だけスキップ
			continue;
		}
		//数字が読めなければstofが例外を投げる
		values[row[0]] = std::stof(row[1]);
	}

	//名前から値を取り出す//無ければ例外を投げて読み込みを失敗にする
	auto get = [&values](const char* name)
	{
		auto it = values.find(name);
		if (it == values.end())
		{
			throw std::runtime_error(std::string("PlayerParam.csv is missing ") + name);
		}
		return it->second;
	};
	//フレーム数などの整数//小数で書かれていても四捨五入する
	auto getInt = [&get](const char* name)
	{
		return static_cast<int>(std::lround(get(name)));
	};

	//移動
	accelFrame = getInt("AccelFrame");
	decelFrame = getInt("DecelFrame");
	rotateRate = get("RotateRate");

	//ジャンプ
	jumpPower = get("JumpPower");
	gravity = get("Gravity");
	airMoveSpeedScale = get("AirMoveSpeedScale");

	//回避
	dodgeFrame = getInt("DodgeFrame");
	dodgeSpeedScale = get("DodgeSpeedScale");
	justDodgeFrame = getInt("JustDodgeFrame");
	justDodgeRadius = get("JustDodgeRadius");

	//ウィッチタイム
	witchTimeFrame = getInt("WitchTimeFrame");
	witchTimeScale = get("WitchTimeScale");

	//被弾
	hitInvincibleFrame = getInt("HitInvincibleFrame");

	//入力
	inputBufferFrame = getInt("InputBufferFrame");

	//攻撃
	hitForwardOffset = get("HitForwardOffset");
	hitHeight = get("HitHeight");
	maxMoveScale = get("MaxMoveScale");
	stopMargin = get("StopMargin");
	witchHandForwardOffset = get("WitchHandForwardOffset");
	witchHandPortalBack =get("WitchHandPortalBack");
	witchHandPortalSide = get("WitchHandPortalSide");
	witchHandPortalHeight = get("WitchHandPortalHeight");
	witchLegPortalHeight = get("WitchLegPortalHeight");

	//長押しで撃つ弾
	shotInterval = getInt("ShotInterval");
	bulletSpeed = get("BulletSpeed");
	bulletDamage = getInt("BulletDamage");
	bulletRadius = get("BulletRadius");
}
