#pragma once
#include "PlayerStateBase.h"
#include "../../../../DataLoader/AttackTable.h"

//プレイヤーの攻撃状態
//攻撃の違いはCSVのデータで表すので、どの攻撃もこのクラス一つで処理する
class PlayerStateAttack : public PlayerStateBase
{
public:
	PlayerStateAttack(Player& player, AttackType type);

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Attack; }
#ifdef _DEBUG
	std::string GetDebugInfo() const override;
#endif

private:
	//今攻撃判定が出ているか
	bool IsHitActive() const;

	//今の攻撃のデータ
	//CSVを読み直してもこの攻撃の途中で中身が消えないようにコピーで持つ
	AttackTable::AttackInfo m_info;
	//攻撃を始めてからのフレーム数
	int m_frame = 0;
	//この攻撃の番号(コンボの次の攻撃は別の番号になる)
	int m_attackId = 0;
	//予約されている次の攻撃(入力がなければ空)
	std::optional<AttackType> m_reservedNext;
};
