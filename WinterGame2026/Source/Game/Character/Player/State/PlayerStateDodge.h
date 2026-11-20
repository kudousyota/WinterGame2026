#pragma once
#include "PlayerStateBase.h"

class PlayerStateDodge : public PlayerStateBase
{
public:
	using PlayerStateBase::PlayerStateBase;

	void Enter() override;
	void Update() override;
	void Exit() override;
	StateKind GetKind() const override { return StateKind::Dodge; }

private:
	//回避する方向
	Vector3 m_dir;
	//回避を始めてからのフレーム数
	int m_frame = 0;
};
