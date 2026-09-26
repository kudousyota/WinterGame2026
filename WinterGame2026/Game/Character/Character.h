#pragma once
#include "Vector3.h"
#
class Character
{
public:
	Character();
	virtual~Character();

	virtual void Init();
	virtual void Update();
	virtual void Draw();

	//ダメージ
	virtual void Damage(int damage);

	//死亡判定
	bool IsDead()const;

protected:
	void LoadStatus();

protected:
	//位置
	Vector3 m_pos = Vector3(0.0f,0.0f,0.0f);
	//前
	Vector3 m_forward = Vector3(0.0f, 0.0f, 0.0f);
	//向き
	Vector3 m_velocity = Vector3(0.0f, 0.0f, 0.0f);
	//スピード
	float m_speed = 0.0f;
	//重力
	float m_gravity = 0.0f;

	//ステータス
	//HP
	int m_hp = 0;
	//HPの最大値
	int m_maxhp = 0;
	//攻撃力
	//int m_attackPower;//たぶんアタックデータの方に入れる
	
	//状態
	//今地面にいるか
	bool m_isGround = false;
	//今ダメージを食らっているか
	bool m_isDamage = false;
	//無敵中
	bool m_isInvincible = false;
	//死亡しているか
	bool m_isDead = false;

};

