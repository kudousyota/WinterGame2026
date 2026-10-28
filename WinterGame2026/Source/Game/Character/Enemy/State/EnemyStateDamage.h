#pragma once
#include "EnemyStateBase.h"

//敵の被弾状態(のけぞり・吹っ飛び)
//被弾アニメーションが終わるまで動かず、終わったら待機に戻る
//吹っ飛ぶ攻撃のときは、のけぞりながら後ろへ下がる
//のけぞり中にもう一度当たると、最初からのけぞり直す
//運ぶ攻撃(竜巻旋風脚など)のときは、のけぞっている間ずっと攻撃してきた相手の目の前についていく
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