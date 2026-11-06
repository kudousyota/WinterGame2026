#include "Damage.h"
#include "../DataLoader/AttackData.h"
#include "Character/Character.h"

int Damage::Calc(const AttackData& attack, const Character& attacker, const Character& target, const Vector3& start, const Vector3& end)
{
	//攻撃ごとのダメージ
	float damage = static_cast<float>(attack.GetDamage());
	//当たった場所による倍率
	damage *= target.GetDamageRate(start, end, attack.GetRadius());

	return static_cast<int>(damage);
}
