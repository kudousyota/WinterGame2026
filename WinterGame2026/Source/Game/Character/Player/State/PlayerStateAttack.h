#pragma once
#include "PlayerStateBase.h"
#include "../../../../DataLoader/AttackTable.h"

//プレイヤーの攻撃状態
//攻撃の違いはCSVのデータで表すので、どの攻撃もこのクラス一つで処理する
class PlayerStateAttack : public PlayerStateBase
{
public:
	PlayerStateAttack(Player& player, AttackType type,const char* buttonName);

	void Enter() override;
	void Update() override;
	void Exit()override;
	StateKind GetKind() const override { return StateKind::Attack; }
#ifdef _DEBUG
	std::string GetDebugInfo() const override;
#endif

private:
	//今攻撃判定が出ているか
	bool IsHitActive() const;
	//攻撃中の前進(決めた距離だけ進む)
	void UpdateMove();
	//コンボの締めでウィケッドウィーブを呼び出す
	void SummonWitchHand();
	//呼び出したウィケッドウィーブの攻撃判定を出す
	void UpdateWitchHand();
private:

	//今の攻撃のデータ
	//CSVを読み直してもこの攻撃の途中で中身が消えないようにコピーで持つ
	AttackTable::AttackInfo m_info;
	//攻撃を始めてからのフレーム数
	int m_frame = 0;
	//次の弾を出すまでのフレーム
	int m_shotTimer = 0;
	//この攻撃の番号(コンボの次の攻撃は別の番号になる)
	int m_attackId = 0;
	//攻撃でどのくらい進むか
	float m_moveDistance = 0.0f;
	//予約した攻撃を出したボタンの名前
	const char* m_reservedButton = nullptr;
	//この攻撃を出したボタンの名前(長押しの判定に使う)
	const char* m_buttonName;
	//予約されている次の攻撃(入力がなければ空)
	std::optional<AttackType> m_reservedNext;

	//呼び出したウィケッドウィーブのデータ(呼び出していなければ空)
	std::optional<AttackTable::AttackInfo> m_witchHandInfo;
	//ウィケッドウィーブを呼び出してからのフレーム数
	int m_witchHandFrame = 0;
	//ウィケッドウィーブの攻撃番号(足技とは別の攻撃として当てる)
	int m_witchHandAttackId = 0;
};
