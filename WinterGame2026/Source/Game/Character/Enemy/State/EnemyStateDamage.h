#pragma once
#include "EnemyStateBase.h"

class EnemyStateDamage : public EnemyStateBase
{
public:
	//distance:後ろへ下がる距離(AttackHit.csvのknockback。0ならその場でのけぞる)
	//isCarry:攻撃してきた相手の目の前に引き寄せられて一緒に運ばれるか(AttackHit.csvのcarry)
	explicit EnemyStateDamage(Enemy& enemy, float distance = 0.0f, bool isCarry = false);

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Damage; }

private:
	//下がる方向
	Vector3 m_flyDir = Vector3(0.0f, 0.0f, 0.0f);
	//下がる距離
	float m_distance = 0.0f;
	//前のフレームまでに進んだ割合
	float m_prevT = 0.0f;
	//攻撃してきた相手に運ばれているか
	bool m_isCarry = false;

	//攻撃してきた相手の目の前へ少しずつ寄せる
	void UpdateCarry();
};