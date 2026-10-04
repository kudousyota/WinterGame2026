#pragma once
#include "Vector3.h"
#include "StateBase.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "../../System/Animation.h"
#include "../../Common/CharacterType.h"
#include "../../DataLoader/AttackData.h"

//攻撃の申請
//攻撃ステートは判定をせず、これをキャラクターに渡すだけにする
//判定はGameSceneが全員の移動が終わった後にまとめて行う
struct AttackRequest
{
	//ダメージ・半径・誰の攻撃か
	AttackData data;
	//攻撃1回ごとの番号(同じ攻撃が何度も当たらないようにするのに使う)
	int attackId = 0;
	//足元からキャラクターの前方への距離
	float forwardOffset = 0.0f;
	//足元からの高さ
	float height = 0.0f;
};

class Character
{
public:
	Character();
	virtual~Character();

	virtual void Init();
	virtual void Update();
	//当たり判定の押し出しなどが終わった後に呼ばれる
	//確定した位置・向きをモデルに反映する
	virtual void LateUpdate() {}
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

	//水平方向の速度を0にする(Y方向の速度は維持)
	void StopMove();

	//ステートから操作するためのアクセサ
	const Vector3& GetPos()const { return m_pos; }
	void SetPosition(const Vector3& pos) { m_pos = pos; }
	const Vector3& GetVelocity()const { return m_velocity; }
	void SetVelocity(const Vector3& velocity) { m_velocity = velocity; }
	const Vector3& GetForward()const { return m_forward; }
	float GetSpeed()const { return m_speed; }
	int GetHp()const { return m_hp; }
	int GetMaxHp()const { return m_maxhp; }
	bool IsGround()const { return m_isGround; }
	void SetInvincible(bool isInvincible) { m_isInvincible = isInvincible; }
	//キャラクタータイプを取得
	virtual CharacterType GetCharacterType()const = 0;
	//アニメーション
	Animation& GetAnim() { return m_anim; }

	//体の位置(足元基準)
	//身長(足元から頭のてっぺんまで)
	float GetHeight()const { return m_height; }
	//体の中心
	Vector3 GetCenterPos()const { return Vector3(m_pos.x, m_pos.y + m_height * 0.5f, m_pos.z); }
	//頭のてっぺん
	Vector3 GetHeadPos()const { return Vector3(m_pos.x, m_pos.y + m_height, m_pos.z); }

	//当たり判定
	//カプセル軸の下端(足元から半径分上げた位置)
	Vector3 GetCollisionPosition()const { return Vector3(m_pos.x, m_pos.y + m_collisionRadius, m_pos.z); }
	//カプセル軸の長さ(身長から上下の半球分を引いた長さ)
	//身長が直径より小さい場合は球として扱う
	float GetCollisionHeight()const
	{
		float axisLength = m_height - m_collisionRadius * 2.0f;
		return (axisLength > 0.0f) ? axisLength : 0.0f;
	}
	//カプセルの半径
	float GetCollisionRadius()const { return m_collisionRadius; }
	//ジャスト回避判定の半径(通常の当たり判定より少し大きくする)
	float GetJustDodgeRadius()const { return m_justDodgeRadius; }
	//ジャスト回避の受付中か
	bool IsJustDodgeWindow()const { return m_isJustDodgeWindow; }

	//攻撃を受けられる状態か(無敵中・死亡中は受けない)
	bool CanBeHit()const { return !m_isInvincible && !m_isDead; }
	//攻撃を受けたときに呼ばれる(damageは補正を計算した後の最終ダメージ)
	virtual void OnHit(const AttackData& attackData, int damage);
	//ジャスト回避に成功したときに呼ばれる
	virtual void OnJustDodge() {}

	//攻撃する側
	//攻撃を始めるたびに新しい番号を発行する
	int IssueAttackId() { return ++m_attackIdCounter; }
	//攻撃を申請する(判定はGameSceneが行う)
	void RequestAttack(const AttackRequest& request) { m_attackRequests.push_back(request); }
	//このフレームに申請された攻撃を取り出す(取り出したら空になる)
	std::vector<AttackRequest> TakeAttackRequests();
	//申請された攻撃の判定位置
	Vector3 GetAttackPos(const AttackRequest& request)const;

	//攻撃を受ける側
	//この攻撃はもう受けたか
	bool HasReceivedAttack(const Character* attacker, int attackId)const;
	//この攻撃を受けたことを記録する
	void MarkAttackReceived(const Character* attacker, int attackId);

	//読み直したステータスを反映する(今のHPは残す)
	void ReloadStatus();

#ifdef _DEBUG
	//デバッグ表示用のステート情報
	std::string GetDebugStateText()const;
#endif

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
	//State.csvでの名前(読み直しに使う)
	std::string m_statusName;

	//攻撃
	//発行した攻撃番号
	int m_attackIdCounter = 0;
	//このフレームに申請された攻撃
	std::vector<AttackRequest> m_attackRequests;
	//攻撃してきた相手ごとに、最後に受けた攻撃番号を覚えておく(相手は所有しない)
	std::unordered_map<const Character*, int> m_lastReceivedAttackId;

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

	//体の大きさ・当たり判定(派生クラスで設定する)
	//身長(足元から頭のてっぺんまで)
	float m_height = 0.0f;
	//カプセルの半径
	float m_collisionRadius = 0.0f;
	//ジャスト回避判定の半径
	float m_justDodgeRadius = 0.0f;

};

