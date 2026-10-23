#pragma once
#include "EnemyStateBase.h"

//敵の被弾状態(のけぞり)
//被弾アニメーションが終わるまで動かず、終わったら待機に戻る
//のけぞり中にもう一度当たると、最初からのけぞり直す
class EnemyStateDamage : public EnemyStateBase
{
public:
	using EnemyStateBase::EnemyStateBase;

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Damage; }
};
