#pragma once
#include "Vector3.h"
#include "State/StateBase.h"
#include <memory>
#include <string>
#include "../../System/Animation.h"
#include "../../Comon/CharacterType.h"
class AttackData;
class Character
{
public:
	//当たり判定用のカプセル
	struct Capsule
	{
		//下の球の中心
		Vector3 start;
		//上の球の中心
		Vector3 end;
		//半径
		float radius = 0.0f;
	};

	Character();
	virtual~Character();

	virtual void Init();
	virtual void Update();
	virtual void Draw();

	//ダメージ
	virtual void Damage(int damage);
	//当たり判定から呼ばれるダメージ処理
	//ジャスト回避受付中にダメージ0で呼ばれた場合はジャスト回避成功として扱う
	virtual void ApplyDamage(int damage);
	//攻撃が当たったときに呼ばれる
	virtual void OnHit(const AttackData& attackData);

	//死亡判定
	bool IsDead()const;

	//ステートを変更する
	//Update中に今のステートを破棄しないよう、実際の切り替えはUpdateStateの中で行う
	void ChangeState(std::unique_ptr<StateBase> nextState);
	//現在のステートの種類を取得
	StateBase::StateKind GetStateKind()const;

	//水平方向の速度を0にする(Y方向の速度は維持)
	void StopMove();

	//ステートから操作するためのアクセサ
	const Vector3& GetPos()const { return m_pos; }
	const Vector3& GetVelocity()const { return m_velocity; }
	void SetVelocity(const Vector3& velocity) { m_velocity = velocity; }
	const Vector3& GetForward()const { return m_forward; }
	float GetSpeed()const { return m_speed; }
	bool IsGround()const { return m_isGround; }
	void SetInvincible(bool isInvincible) { m_isInvincible = isInvincible; }
	//キャラクタータイプを取得
	virtual CharacterType GetCharacterType()const = 0;

	//当たり判定用のアクセサ
	//位置を設定する(押し出しなどで使う)
	void SetPosition(const Vector3& pos) { m_pos = pos; }
	//現在位置のカプセルを取得
	Capsule GetCapsule()const { return GetCapsuleAt(m_pos); }
	//指定した足元位置にいるときのカプセルを取得(押し出し計算の途中などで使う)
	Capsule GetCapsuleAt(const Vector3& footPos)const;
	//体の中心(ロックオン・エフェクト・狙う位置などに使う)
	Vector3 GetCenterPos()const { return m_pos + Vector3(0.0f, m_collisionHeight * 0.5f, 0.0f); }
	//カプセルの半径
	float GetCollisionRadius()const { return m_collisionRadius; }
	//カプセルの高さ(足元から頭のてっぺんまでの全体の高さ)
	float GetCollisionHeight()const { return m_collisionHeight; }
	//ジャスト回避の判定半径
	float GetJustDodgeRadius()const { return m_justDodgeRadius; }
	//ジャスト回避の受付中か
	bool IsJustDodgeWindow()const { return m_isJustDodgeWindow; }
	void SetJustDodgeWindow(bool isJustDodgeWindow) { m_isJustDodgeWindow = isJustDodgeWindow; }
	//無敵中か
	bool IsInvincible()const { return m_isInvincible; }
	//アニメーション
	Animation& GetAnim() { return m_anim; }

protected:
	
	void LoadStatus(const std::string& name);

	//ジャスト回避が成功したときに呼ばれる(処理は派生クラスで書く)
	virtual void OnJustDodge() {}

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
	//ジャスト回避の受付中か
	bool m_isJustDodgeWindow = false;

	//当たり判定
	//カプセルの半径
	float m_collisionRadius = 0.0f;
	//カプセルの高さ(足元から頭のてっぺんまでの全体の高さ)
	float m_collisionHeight = 0.0f;
	//ジャスト回避の判定半径
	float m_justDodgeRadius = 0.0f;

};

