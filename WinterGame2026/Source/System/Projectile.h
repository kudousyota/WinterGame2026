#pragma once
#include "Vector3.h"
#include "../DataLoader/AttackData.h"
#include "../System/CollisionManager.h"

//敵が使う投擲物や球などの基底クラス
class Projectile
{
public:
	//引数は変更しないからconst参照で受け取る
	//maxLifeFrameが0以下なら寿命なし
	Projectile(const Vector3& pos,const Vector3& forward,float speed,const AttackData& attack,float maxLifeFrame = 0.0f);
	virtual~Projectile() = default;

	void SetPos(const Vector3& pos) { m_pos = pos; }
	void SetForward(const Vector3& forwrd) { m_forward = forwrd; }

	//当たり判定に使う
	const Vector3& GetPos()const { return m_pos; }
	const Vector3& GetPrevPos()const { return m_prevPos; }
	const AttackData& GetAttackData()const { return m_attackData; }

	//timeScaleはウィッチタイムなどの時間倍率
	virtual void Update(float timeScale) = 0;
	virtual void Draw() = 0;

	//このフレームに当たった相手を調べる
	//標準は前の位置から今の位置までのカプセルで調べるので、速い弾もすり抜けない
	//形が違う飛び道具はこれを上書きする
	virtual std::vector<CollisionManager::AttackHit> CheckHits(CollisionManager& collision)const;
	//相手に当たったときに呼ばれる
	//isJustDodgeがtrueならジャスト回避された
	//標準は弾と同じように消える。消えずに残る飛び道具はこれを上書きする
	virtual void OnHitTarget(bool isJustDodge);
#ifdef _DEBUG
	//当たり範囲などを描く
	virtual void DrawDebug()const {}
#endif

	//消す
	void Kill() { m_isDead = true; }
	bool IsDead()const { return m_isDead; }
protected:
	//寿命を進めて、最大寿命を超えたら消す
	void UpdateLife(float timeScale);

	//位置
	Vector3 m_pos;
	//前の位置
	Vector3 m_prevPos;
	//前方
	Vector3 m_forward;
	//スピード
	float m_speed;
	//生存フレーム
	float m_lifeFrame;
	//最大生存フレーム
	float m_maxLifeFrame;
	//まだあるか
	bool m_isDead;
	//アタックデータ
	AttackData m_attackData;
};