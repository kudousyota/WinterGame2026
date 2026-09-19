#pragma once
#include "Vector3.h"

class Character
{
public:
	Character();
	~Character();

	virtual void Init();
	virtual void Update();
	virtual void Draw();

	//ダメージ
	virtual void Damage(int damage);

	//死亡判定
	bool IsDead()const;

private:
	//位置
	Vector3 m_pos;
	//前
	Vector3 m_forward;
	//向き
	Vector3 m_velocity;
	//スピード
	float m_speed;
	//重力
	float m_gravity;

	//ステータス
	//HP
	int m_hp;
	//HPの最大値
	int m_maxhp;
	//攻撃力
	//int m_attackPower;//たぶんアタックデータの方に入れる
	
	//状態
	//今地面にいるか
	bool m_isGround;
	//今ダメージを食らっているか
	bool m_isDamage;
	//無敵中
	bool m_isInvincible;
	//死亡しているか
	bool m_isDead;

};

