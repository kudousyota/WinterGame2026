#pragma once
#include "Character.h"

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

	//キャラクタータイプを取得
	CharacterType GetCharacterType()const override { return CharacterType::Enemy; }

	//狙う相手を設定する
	void SetTarget(const Character* target) { m_pTarget = target; }

	//ターゲットまでの距離(XZ平面)
	//ターゲットがいない場合はとても大きい値を返す
	float GetTargetDistance()const;
	//ターゲットの方向へ移動する(Y方向の速度は維持)
	void MoveToTarget(float speedScale);

	//索敵範囲
	float GetSearchRange()const { return m_searchRange; }
	//攻撃範囲
	float GetAttackRange()const { return m_attackRange; }

protected:
	//狙う相手(所有はしない)
	const Character* m_pTarget = nullptr;

	//索敵範囲(派生クラスで設定する)
	float m_searchRange = 0.0f;
	//攻撃範囲(派生クラスで設定する)
	float m_attackRange = 0.0f;
};

