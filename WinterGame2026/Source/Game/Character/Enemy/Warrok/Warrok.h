#pragma once
#include "../Enemy.h"

//近距離で殴ってくる地上のボス
class Warrok : public Enemy
{
public:
	Warrok();
	~Warrok()override;
	void Init()override;
	void Update()override;
	void LateUpdate()override;
	void Draw()override;

	float GetDamageRate(const Vector3& start, const Vector3& end, float radius)const override;

private:
	Vector3 GetModelForward()const;
	Vector3 GetWeakPointPos()const;

	//弱点に当たったか
	bool IsHitWeakPoint(const Vector3& start, const Vector3& end, float radius)const;

	//弱点の目印のエフェクトを出す位置(体に埋もれないよう弱点より少し外側)
	Vector3 GetWeakPointEffectPos()const;
	//弱点の目印のエフェクトを消す(消えるまでフェードする)
	void StopWeakPointEffect();

	int m_modelHandle = -1;
	//弱点の目印のエフェクト(出していないときは-1)
	int m_weakPointEffectHandle = -1;
};
