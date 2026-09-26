#include "Character.h"
Character::Character()
{
}

Character::~Character()
{
}

void Character::Init()
{

}

void Character::Update()
{

}

void Character::Draw()
{

}

void Character::Damage(int damage)
{
	m_hp -= damage;
}

bool Character::IsDead() const
{
	return false;
}
