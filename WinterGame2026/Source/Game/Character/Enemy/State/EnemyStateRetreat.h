#pragma once
#include "EnemyStateBase.h"

//プレイヤーから離れる状態
//体はプレイヤーに向けたまま後ろへ下がり、保つ距離まで離れたら様子見に移る
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