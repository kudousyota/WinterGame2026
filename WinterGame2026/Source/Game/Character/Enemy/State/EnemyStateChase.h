#pragma once
#include "EnemyStateBase.h"

//敵の追跡状態
//ターゲットに向かって歩き、攻撃範囲に入ったら攻撃する
class EnemyStateChase : public EnemyStateBase
{
public:
	using EnemyStateBase::EnemyStateBase;

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Walk; }
};
