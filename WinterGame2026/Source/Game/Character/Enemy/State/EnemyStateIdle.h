#pragma once
#include "EnemyStateBase.h"

//敵の待機状態
//攻撃の後の隙も兼ねていて、一定フレーム待ってから次の行動を決める
class EnemyStateIdle : public EnemyStateBase
{
public:
	using EnemyStateBase::EnemyStateBase;

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Idle; }

private:
	//待機を始めてからのフレーム数(ウィッチタイム中はゆっくり進む)
	float m_frame = 0.0f;
};
