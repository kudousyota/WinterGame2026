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

	//近ければ殴る。離れていても、ジャンプ攻撃か突進が使えるならそれで攻撃する
	bool CanStartAttack(float distance)const override;
	//ジャンプ攻撃と突進は専用のステートにする
	std::unique_ptr<StateBase> CreateAttackState()override;

#ifdef _DEBUG
	//背中の弱点の当たり判定を球で描く
	void DrawDebugWeakPoint()const override;
#endif

protected:
	//離れていたらジャンプ攻撃か突進、近ければパンチとスワイプから選ぶ
	AttackType SelectAttack()override;

private:
	//パンチとスワイプからランダムに選ぶ
	//同じ攻撃が続きすぎないようにする
	AttackType SelectMeleeAttack();
	Vector3 GetModelForward()const;
	Vector3 GetWeakPointPos()const;
	//弱点の位置を背中のボーンに合わせる(モデルの位置とアニメーションが決まった後に呼ぶ)
	void UpdateWeakPointPos();

	//弱点に当たったか
	bool IsHitWeakPoint(const Vector3& start, const Vector3& end, float radius)const;

	//弱点の目印のエフェクトを出す位置(体に埋もれないよう弱点より少し外側)
	Vector3 GetWeakPointEffectPos()const;
	//弱点の目印のエフェクトを消す(消えるまでフェードする)
	void StopWeakPointEffect();

	int m_modelHandle = -1;
	//同じ攻撃を何回続けて出したか
	int m_sameAttackCount = 0;
	//最後に出した近接攻撃
	//間にジャンプ攻撃を挟んでも、パンチとスワイプの交互の流れを覚えておく
	AttackType m_lastMeleeAttack = AttackType::WarrokPunch;
	//次にジャンプ攻撃を使えるまでのフレーム数
	//0以下なら使える
	float m_jumpAttackCooldown = 0.0f;
	//次に突進を使えるまでのフレーム数
	//0以下なら使える
	float m_chargeCooldown = 0.0f;
	//弱点の目印のエフェクト(出していないときは-1)
	int m_weakPointEffectHandle = -1;
	//弱点の基準にする背中のボーン(見つからなければ-1)
	int m_weakPointFrame = -1;
	//ボーンから見た弱点の中心(ボーンが回転しても背中の同じ場所についていく)
	Vector3 m_weakPointLocalPos;
	//弱点の中心(LateUpdateで更新する)
	Vector3 m_weakPointPos;
};
