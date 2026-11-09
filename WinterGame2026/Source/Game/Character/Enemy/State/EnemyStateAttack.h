#pragma once
#include "EnemyStateBase.h"
#include "../../../../DataLoader/AttackTable.h"

//敵の攻撃状態
//攻撃の中身はプレイヤーと同じくAttackHit.csvとAttackMotion.csvのデータで表す
//判定が出るまでが予兆で、プレイヤーにとっての回避のタイミングになる
class EnemyStateAttack : public EnemyStateBase
{
public:
	explicit EnemyStateAttack(Enemy& enemy);

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
	//攻撃判定の出る位置からターゲットの体の中心へ向かう方向
	Vector3 CalcAimDir() const;
	//今の攻撃判定の長さ
	float CalcHitLength() const;
	//予兆エフェクトを止める
	void StopTelegraph();

	//今の攻撃のデータ(CSVを読み直しても攻撃の途中で中身が消えないようにコピーで持つ)
	AttackTable::AttackInfo m_info;
	//攻撃を始めてからのフレーム数
	float m_frame = 0.0f;
	//この攻撃の番号
	int m_attackId = 0;
	//次に当たり直すフレーム
	float m_nextHitFrame = 0.0f;
	//狙う方向
	Vector3 m_aimDir = Vector3(0.0f, 0.0f, 0.0f);
	//再生中の予兆エフェクトのハンドル
	int m_telegraphHandle = -1;
};