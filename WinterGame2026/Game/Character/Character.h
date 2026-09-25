#pragma once
#include <string>
#include "Vector3.h"

//キャラクターの基底クラス
//プレイヤーや敵はこのクラスを継承して作る
class Character
{
public:
	//name : State.csvのキャラクター名
	Character(const std::string& name);
	virtual ~Character();

	virtual void Init();
	virtual void Update();
	virtual void Draw();

	//ダメージ
	virtual void Damage(int damage);

	//死亡判定
	bool IsDead()const;

	//取得
	const Vector3& GetPos()const { return m_pos; }
	const Vector3& GetForward()const { return m_forward; }
	int GetHp()const { return m_hp; }
	int GetMaxHp()const { return m_maxhp; }
	bool IsGround()const { return m_isGround; }
	bool IsInvincible()const { return m_isInvincible; }

protected:
	//モデルの読み込み
	void LoadModel(const char* path);
	//重力と移動量を位置に反映する
	void UpdateMove();
	//無敵時間の更新
	void UpdateInvincible();
	//死亡した瞬間に呼ばれる(派生クラスで演出などを書く)
	virtual void OnDead() {}

protected:
	//キャラクター名(ステータスの検索に使う)
	std::string m_name;
	//モデルハンドル
	int m_modelHandle;

	//位置
	Vector3 m_pos;
	//前
	Vector3 m_forward;
	//移動量
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
	//無敵の残りフレーム
	int m_invincibleFrame;
	//死亡しているか
	bool m_isDead;

};
