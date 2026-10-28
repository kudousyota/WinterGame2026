#include "Damage.h"
#include "../DataLoader/AttackData.h"
#include "Character/Character.h"

int Damage::Calc(const AttackData& attack, const Character& attacker, const Character& target)
{
	//攻撃ごとのダメージ(AttackHit.csv)
	float damage = static_cast<float>(attack.GetDamage());

	//補正は今は何もない。必要になったらここに足す

	return static_cast<int>(damage);
}
