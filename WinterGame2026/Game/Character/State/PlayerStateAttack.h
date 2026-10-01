#pragma once
#include "PlayerStateBase.h"
#include "../../../DataLoader/AttackTable.h"

//プレイヤーの攻撃状態
//攻撃の違いはCSVのデータで表すので、どの攻撃もこのクラス一つで処理する
class PlayerStateAttack : public PlayerStateBase
{
public:
	PlayerStateAttack(Player& player, AttackType type);

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Attack; }

private:
	//今攻撃判定が出ているか
	bool IsHitActive() const;

	//今の攻撃のデータ
	const AttackTable::AttackInfo& m_info;
	//攻撃を始めてからのフレーム数
	int m_frame = 0;
	//次の攻撃の入力が予約されているか
	bool m_isComboReserved = false;
};
