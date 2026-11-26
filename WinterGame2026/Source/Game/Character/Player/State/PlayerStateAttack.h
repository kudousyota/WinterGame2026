#pragma once
#include "PlayerStateBase.h"
#include "../../../../DataLoader/AttackTable.h"

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
	//呼び出した後の攻撃判定はPlayerが持つWickedWeaveが出す
	void SummonWitchHand();
	//この攻撃の攻撃判定を申請する
	void RequestHit();

	//長押しで撃ち始めたときに、弾が出る位置へ魔法陣を出す
	void OpenShotCircle();
	//撃ち終わったら魔法陣をフェードして消す
	void CloseShotCircle();
private:

	//今の攻撃のデータ
	//CSVを読み直してもこの攻撃の途中で中身が消えないようにコピーで持つ
	AttackTable::AttackInfo m_info;
	//攻撃を始めてからのフレーム数
	int m_frame = 0;
	//次の弾を出すまでのフレーム
	int m_shotTimer = 0;
	//撃っている間に出す魔法陣のエフェクト(出していなければ-1)
	int m_shotCircleHandle = -1;
	//この攻撃の番号(コンボの次の攻撃は別の番号になる)
	int m_attackId = 0;
	//多段ヒットの何段目か//攻撃番号を新しくするたびに増える
	int m_hitIndex = 1;
	//攻撃でどのくらい進むか
	float m_moveDistance = 0.0f;
	//予約した攻撃を出したボタンの名前
	const char* m_reservedButton = nullptr;
	//この攻撃を出したボタンの名前(長押しの判定に使う)
	const char* m_buttonName;
	//予約されている次の攻撃(入力がなければ空)
	std::optional<AttackType> m_reservedNext;

	//ウィケッドウィーブをもう呼び出したか
	//長押しで止まっている間も呼び出すフレームのままなので、何度も呼び出さないようにする
	bool m_isWitchHandSummoned = false;

	bool m_isAir = false;
	//地面まで落ちる攻撃(かかと落とし)の途中か。着地したらfalseになる
	bool m_isFalling = false;
};
