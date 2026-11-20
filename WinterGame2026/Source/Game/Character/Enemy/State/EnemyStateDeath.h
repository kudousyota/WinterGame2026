#pragma once
#include "EnemyStateBase.h"

class EnemyStateDeath : public EnemyStateBase
{
public:
	using EnemyStateBase::EnemyStateBase;

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Death; }
#ifdef _DEBUG
	std::string GetDebugInfo() const override;
#endif

private:
	//倒れるアニメーションが終わってからのフレーム数(ウィッチタイム中はゆっくり進む)
	float m_afterAnimFrame = 0.0f;
};
