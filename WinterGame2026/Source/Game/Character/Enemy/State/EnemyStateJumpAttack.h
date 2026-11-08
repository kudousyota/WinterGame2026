#pragma once
#include "EnemyStateBase.h"
#include "../../../../DataLoader/AttackTable.h"

//敵のジャンプ攻撃
//その場で跳んで、着地で攻撃判定と衝撃波を出す
//近づいてはこないので、離れたプレイヤーには衝撃波で攻撃する
//溜め、跳ぶ、着地、硬直の順に進む
//CSVの値の使い方
//  windupFrame・windupAnimScale:溜めをゆっくり再生する
//  moveStartFrame:跳ぶフレーム。ここまではプレイヤーの方を向き続ける
//  moveEndFrame:着地するフレーム。デバッグ表示に使う
//  hitStartFrame・hitEndFrame:着地の攻撃判定。足元を中心にした球
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
