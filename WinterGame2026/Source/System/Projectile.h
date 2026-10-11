#pragma once
#include "Vector3.h"
#include "../DataLoader/AttackData.h"
//敵が使う投擲物や球などの基底クラス
class Projectile
{
public:
	//引数は変更しないからconst参照で受け取る
	//maxLifeFrameが0以下なら寿命なし(Killされるまで残る)
	Projectile(const Vector3& pos,const Vector3& forward,float speed,const AttackData& attack,float maxLifeFrame = 0.0f);
	virtual~Projectile() = default;

	void SetPos(const Vector3& pos) { m_pos = pos; }
	void SetForward(const Vector3& forwrd) { m_forward = forwrd; }

	//当たり判定に使う(判定はGameSceneが行う)
	const Vector3& GetPos()const { return m_pos; }
	const Vector3& GetPrevPos()const { return m_prevPos; }
	const AttackData& GetAttackData()const { return m_attackData; }

	//timeScaleはウィッチタイムなどの時間倍率
	virtual void Update(float timeScale) = 0;
	virtual void Draw() = 0;

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
	//生存フレーム(時間倍率で進むのでfloat)
	float m_lifeFrame;
	//最大生存フレーム
	float m_maxLifeFrame;
	//まだあるか
	bool m_isDead;
	//アタックデータ
	AttackData m_attackData;
};