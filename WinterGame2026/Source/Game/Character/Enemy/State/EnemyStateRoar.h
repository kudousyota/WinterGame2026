#pragma once
#include "EnemyStateBase.h"

//その場で叫ぶ//ボスの登場演出で使う
//叫び終わったら待機に戻る
//動き出すかどうかは狙う相手がいるかで決まるので、演出中はEnemy::SetHoldで止めておく
class EnemyStateRoar : public EnemyStateBase
{
public:
	using EnemyStateBase::EnemyStateBase;

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Roar; }
};
