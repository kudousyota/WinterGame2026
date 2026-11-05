#pragma once
#include "../Enemy.h"

//近距離で殴ってくる地上のボス
class Warrok : public Enemy
{
public:
	Warrok();
	~Warrok()override;
	void Init()override;
	void Update()override;
	void LateUpdate()override;
	void Draw()override;

private:
	int m_modelHandle = -1;
};
