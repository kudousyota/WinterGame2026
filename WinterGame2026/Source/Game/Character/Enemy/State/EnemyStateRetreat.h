#pragma once
#include "EnemyStateBase.h"

class EnemyStateRetreat : public EnemyStateBase
{
public:
	using EnemyStateBase::EnemyStateBase;

	void Enter() override;
	void Update() override;
	void Exit() override;
	StateKind GetKind() const override { return StateKind::Retreat; }

private:
	//離れ始めてからのフレーム数(壁際で離れられないときに打ち切るのに使う)
	float m_frame = 0.0f;
};