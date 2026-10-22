#pragma once
#include "../Common/CharacterType.h"
//値を表すだけなので処理はない
enum class AttackType
{
	Punch,
	Rush,
	Kick,
	JumpKick,
	Punching,
	Kicking,
	MmaKick,
	SideKick,
	RotateKick,
	HurricaneKick,
	LegSweep,
	MeleeKick,
	//空中攻撃
	AirPunch,
	AirPunching,
	AirRush,
	AirSkyKick,
	Attack,
	Breath,
	WitchHand,
};

class AttackData
{
public:
	//誰か//攻撃タイプ//攻撃力//半径
	AttackData(CharacterType attacker, AttackType type, int damage, float radius);

	//攻撃したのは誰なのか
	void SetAttacker(CharacterType attacker) { m_attacker = attacker; }
	CharacterType GetAttacker()const { return m_attacker; }

	//攻撃のタイプ
	void SetAttackType(AttackType type) { m_attackType = type; }
	AttackType GetAttackType()const { return m_attackType; }

	//ダメージ系
	void SetDamage(int damage) { m_damage = damage; }
	int GetDamage()const { return m_damage; }
	//攻撃の半径
	float GetRadius() const { return m_radius; }

	

private:
	CharacterType m_attacker = CharacterType::Player;
	AttackType m_attackType = AttackType::Punch;
	int m_damage = 0;
	float m_radius = 0;

};

