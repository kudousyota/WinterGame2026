#pragma once
#include "EnemyStateBase.h"

class EnemyStateObserve : public EnemyStateBase
{
public:
	using EnemyStateBase::EnemyStateBase;

	void Enter() override;
	void Update() override;
	void Exit() override;
	StateKind GetKind() const override { return StateKind::Observe; }

private:
	//様子見を始めてからのフレーム数
	float m_frame = 0.0f;
	//回る向き(1か-1)
	float m_side = 1.0f;
};