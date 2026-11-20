#pragma once
#include "EnemyStateBase.h"
#include "../../../../DataLoader/AttackTable.h"

class EnemyStateJumpAttack : public EnemyStateBase
{
public:
	explicit EnemyStateJumpAttack(Enemy& enemy);

	void Enter() override;
	void Update() override;
	void Exit() override;
	StateKind GetKind() const override { return StateKind::Attack; }
#ifdef _DEBUG
	std::string GetDebugInfo() const override;
#endif

private:
	//今攻撃判定が出ているか
	bool IsHitActive() const;
	//予兆エフェクトを止める。再生していなければ何もしない
	void StopTelegraph();

	//今の攻撃のデータ
	//CSVを読み直しても攻撃の途中で中身が消えないようにコピーで持つ
	AttackTable::AttackInfo m_info;
	//攻撃を始めてからのフレーム数。ウィッチタイム中はゆっくり進む
	float m_frame = 0.0f;
	//この攻撃の番号
	int m_attackId = 0;
	//もう衝撃波を出したか
	bool m_hasShockwave = false;
	//再生中の予兆エフェクトのハンドル。再生していないときは-1
	int m_telegraphHandle = -1;
};
