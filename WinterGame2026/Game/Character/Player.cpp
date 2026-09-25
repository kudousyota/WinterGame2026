#include "Player.h"

Player::Player() :
	Character("Player")
{
	LoadModel("Data/Model/Player.mv1");
}

Player::~Player()
{
}

void Player::Init()
{
	Character::Init();
}

void Player::Update()
{
	//‚±‚±‚É“ü—Íˆ—‚ğ‘‚­

	Character::Update();
}
