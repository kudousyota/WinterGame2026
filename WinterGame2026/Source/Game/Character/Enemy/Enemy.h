#pragma once
#include "../Character.h"

//敵の基底クラス
//どの敵にも共通する処理だけを持つ(敵ごとの処理は派生クラスに書く)
class Enemy : public Character
{
public:
	Enemy();
	~Enemy()override;
	void Init()override;
	void Update()override;
	void Draw()override;

	CharacterType GetCharacterType()const override { return CharacterType::Enemy; }

	//狙う相手を設定する
	void SetTarget(const Character* target) { m_pTarget = target; }

	//ターゲットまでの距離(XZ平面)
	//ターゲットがいない場合はとても大きい値を返す
	float GetTargetDistance()const;
	//ターゲットの方向へ移動する(Y方向の速度は維持)
	void MoveToTarget(float speedScale);
	//ターゲットの方を向く
	void FaceTarget();

	//敵の時間の倍率(ゲーム全体の倍率×敵専用の倍率。ウィッチタイム中は遅くなる)
	//敵のステートのフレーム数・移動・アニメーションはすべてこれを掛けて進める
	float GetTimeScale()const;

	//索敵範囲
	float GetSearchRange()const { return m_searchRange; }
	//攻撃範囲
	float GetAttackRange()const { return m_attackRange; }

	//ステートから使うアニメーションの名前と攻撃の種類(派生クラスで設定する)
	const std::string& GetIdleAnimName()const { return m_idleAnimName; }
	const std::string& GetMoveAnimName()const { return m_moveAnimName; }
	AttackType GetAttackType()const { return m_attackType; }

protected:
	//狙う相手(所有はしない)
	const Character* m_pTarget = nullptr;

	//索敵範囲(派生クラスで設定する)
	float m_searchRange = 0.0f;
	//攻撃範囲(派生クラスで設定する)
	float m_attackRange = 0.0f;

	//待機のアニメーション
	std::string m_idleAnimName;
	//移動のアニメーション
	std::string m_moveAnimName;
	//攻撃の種類(フレームやダメージはcsvに書く)
	AttackType m_attackType = AttackType::Attack;
};

