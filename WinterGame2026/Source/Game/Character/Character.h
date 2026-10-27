#pragma once
#include "Vector3.h"
#include "Geometry.h"
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
	//カプセル判定の向きと長さ
	Vector3 dir = Vector3(0.0f, 0.0f, 0.0f);
	float length = 0.0f;
};
struct ShotRequest
{
	AttackData data;
	float speed = 0.0f;
	float forwardOffset = 0.0f;
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
	//ステージとの当たり判定の結果を反映する
	void SetGround(bool isGround) { m_isGround = isGround; }
	void SetInvincible(bool isInvincible) { m_isInvincible = isInvincible; }
	void SetJustDodgeWindow(bool isWindow) { m_isJustDodgeWindow = isWindow; }
	//キャラクタータイプを取得
	virtual CharacterType GetCharacterType()const = 0;
	//アニメーション
	Animation& GetAnim() { return m_anim; }

	//体の大きさ・部位の位置(足元基準)
	//身長(足元から頭のてっぺんまで)
	float GetHeight()const { return m_height; }
	//体の中心
	Vector3 GetCenterPos()const { return Vector3(m_pos.x, m_pos.y + m_height * 0.5f, m_pos.z); }
	//頭のてっぺん
	Vector3 GetHeadPos()const { return Vector3(m_pos.x, m_pos.y + m_height, m_pos.z); }

	//当たり判定
	//当たり判定のカプセル(足元から頭のてっぺんまでを包む)
	Geometry::Capsule GetCollisionCapsule()const;
	//ジャスト回避判定のカプセル(当たり判定と同じ軸で半径だけ大きい)
	Geometry::Capsule GetJustDodgeCapsule()const;
	//カプセル軸の長さ(身長から上下の半球分を引いた長さ)
	//身長が直径より小さい場合は球として扱う
	float GetCollisionHeight()const;
	//カプセルの半径
	float GetCollisionRadius()const { return m_collisionRadius; }
	//ジャスト回避判定の半径(通常の当たり判定より少し大きくする)
	float GetJustDodgeRadius()const { return m_justDodgeRadius; }
	//ジャスト回避の受付中か
	bool IsJustDodgeWindow()const { return m_isJustDodgeWindow; }

	//攻撃を受けられる状態か(無敵中・被弾後の無敵中・死亡中は受けない)
	bool CanBeHit()const { return !m_isInvincible && m_hitInvincibleFrame <= 0 && !m_isDead; }
	//攻撃を受けたときに呼ばれる(damageは補正を計算した後の最終ダメージ)
	virtual void OnHit(const AttackData& attackData, int damage);
	//ジャスト回避に成功したときに呼ばれる
	virtual void OnJustDodge() {}

	//攻撃する側
	//攻撃を始めるたびに新しい番号を発行する
	int IssueAttackId() { return ++m_attackIdCounter; }
	//攻撃を申請する(判定はGameSceneが行う)
	void RequestAttack(const AttackRequest& request) { m_attackRequests.push_back(request); }
	
	void RequestShot(const ShotRequest& request) { m_shotRequests.push_back(request); }

	//このフレームに申請された攻撃を取り出す(取り出したら空になる)
	std::vector<AttackRequest> TakeAttackRequests();
	//このフレームに申請された飛び道具を取り出す(取り出したら空になる)
	std::vector<ShotRequest> TakeShotRequests();
	//申請された攻撃の判定位置
	Vector3 GetAttackPos(const AttackRequest& request)const;
	Vector3 GetShotPos(const ShotRequest& request)const;
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

	//位置(足元)
	Vector3 m_pos = Vector3(0.0f,0.0f,0.0f);
	//向いている方向(長さ1)
	Vector3 m_forward = Vector3(0.0f, 0.0f, 0.0f);
	//速度(1フレームに動く量)
	Vector3 m_velocity = Vector3(0.0f, 0.0f, 0.0f);
	//モデルのY軸回転の角度(ラジアン。m_forwardへ少しずつ近づける)
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
	//このフレームに申請された飛び道具
	std::vector<ShotRequest> m_shotRequests;
	//攻撃してきた相手ごとに、最近受けた攻撃番号を覚えておく(相手は所有しない)
	std::unordered_map<const Character*, std::vector<int>> m_receivedAttackIds;

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
	//被弾後の無敵の残りフレーム数(回避の無敵とは別に数える)
	int m_hitInvincibleFrame = 0;
};

