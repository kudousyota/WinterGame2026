#pragma once
#include "EnemyStateBase.h"
#include "../../../../DataLoader/AttackTable.h"

class EnemyStateCharge : public EnemyStateBase
{
public:
	explicit EnemyStateCharge(Enemy& enemy);
	void Enter() override;
	void Update()override;
	void Exit() override;
	StateKind GetKind()const override { return StateKind::Attack; }
#ifdef _DEBUG
	std::string GetDebugInfo() const override;
#endif

private:
	//今攻撃判定が出ているか
	bool IsHitActive() const;
	//突進中か
	bool IsCharging() const;
	//突進を始める瞬間に1回だけ呼ぶ
	void StartCharge();

	//今の攻撃のデータ
	AttackTable::AttackInfo m_info;
	//攻撃を始めてからのフレーム数
	float m_frame = 0.0f;
	//この攻撃の番号//同じ突進が何回も当たらないようにする
	int m_attackId = 0;
	//再生中の予兆エフェクトのハンドル
	int m_telegraphHandle = -1;
	//突進する方向//突進を始めた瞬間に決めてその後は曲がらない
	Vector3 m_chargeDir = Vector3(0.0f, 0.0f, 0.0f);
	//1フレームに進む距離
	float m_chargeSpeed = 0.0f;
	//もう突進を始めたか
	bool m_hasStarted = false;
};

