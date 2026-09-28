#pragma once
#include "Vector3.h"
#include "State/StateBase.h"
#include <memory>
#include <string>
#include "../../System/Animation.h"
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

	//ステートを変更する
	//Update中に今のステートを破棄しないよう、実際の切り替えはUpdateStateの中で行う
	void ChangeState(std::unique_ptr<StateBase> nextState);
	//現在のステートの種類を取得
	StateBase::StateKind GetStateKind()const;

	//ステートから操作するためのアクセサ
	const Vector3& GetVelocity()const { return m_velocity; }
	void SetVelocity(const Vector3& velocity) { m_velocity = velocity; }
	const Vector3& GetForward()const { return m_forward; }
	float GetSpeed()const { return m_speed; }
	bool IsGround()const { return m_isGround; }
	void SetInvincible(bool isInvincible) { m_isInvincible = isInvincible; }
	//アニメーション
	Animation& GetAnim() { return m_anim; }

protected:
	void LoadStatus(const std::string& name);

	//ステートの更新と切り替えを行う
	void UpdateState();

private:
	//予約されているステートがあれば切り替える
	void ApplyNextState();

protected:
	//現在のステート
	std::unique_ptr<StateBase> m_pState;
	//次のステート(ChangeStateで予約される)
	std::unique_ptr<StateBase> m_pNextState;
	Animation m_anim;

	//位置
	Vector3 m_pos = Vector3(0.0f,0.0f,0.0f);
	//前
	Vector3 m_forward = Vector3(0.0f, 0.0f, 0.0f);
	//向き
	Vector3 m_velocity = Vector3(0.0f, 0.0f, 0.0f);
	//今の角度
	float m_angle = 0.0f;
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
	int m_attackPower = 0;//たぶんアタックデータの方に入れる
	
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

