#pragma once
#include "EnemyStateBase.h"

class EnemyStateIdle : public EnemyStateBase
{
public:
	using EnemyStateBase::EnemyStateBase;

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Idle; }

private:
	//待機を始めてからのフレーム数
	float m_frame = 0.0f;
};
