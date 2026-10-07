#pragma once
#include "EnemyStateBase.h"
#include "../../../../DataLoader/AttackTable.h"

//敵の攻撃状態
//攻撃の中身はプレイヤーと同じくAttack.csvのデータで表す
//判定が出るまでのフレームがプレイヤーにとっての回避のタイミングになる
class EnemyStateAttack : public EnemyStateBase
{
public:
	explicit EnemyStateAttack(Enemy& enemy);

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Attack; }
#ifdef _DEBUG
	std::string GetDebugInfo() const override;
#endif

private:
	//今攻撃判定が出ているか
	bool IsHitActive() const;

	//今の攻撃のデータ(CSVを読み直しても攻撃の途中で中身が消えないようにコピーで持つ)
	AttackTable::AttackInfo m_info;
	//攻撃を始めてからのフレーム数(ウィッチタイム中はゆっくり進む)
	float m_frame = 0.0f;
	//この攻撃の番号
	int m_attackId = 0;
};
