#pragma once
#include "Character.h"
class Alien : public Character
{
public:
	Alien();
	~Alien()override;
	void Init()override;
	void Update()override;
	void Draw()override;

	//キャラクタータイプを取得
	CharacterType GetCharacterType()const override { return CharacterType::Enemy; }
private:
	int m_modelHandle = -1;
};