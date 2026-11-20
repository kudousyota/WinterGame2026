#pragma once
#include "EnemyStateBase.h"

class EnemyStateGetUp : public EnemyStateBase
{
public:
	using EnemyStateBase::EnemyStateBase;

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::GetUp; }
private:
	//浮き上がっている途中か
	bool m_isRising = false;
	//浮き上がる目標地点
	float m_tagetY = 0.0f;
};
