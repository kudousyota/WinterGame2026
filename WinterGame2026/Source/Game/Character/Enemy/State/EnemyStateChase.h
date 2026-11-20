#pragma once
#include "EnemyStateBase.h"

class EnemyStateChase : public EnemyStateBase
{
public:
	using EnemyStateBase::EnemyStateBase;

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Walk; }
};
