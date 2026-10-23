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

	//攻撃を受けたらのけぞる
	void OnHit(const AttackData& attackData, int damage)override;

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
	const std::string& GetDamageAnimName()const { return m_damageAnimName; }
	const std::string& GetDeathAnimName()const { return m_deathAnimName; }
	const std::string& GetKnockdownAnimName()const { return m_knockdownAnimName; }
	const std::string& GetGetUpAnimName()const { return m_getUpAnimName; }

	//死亡ステートから使う
	//モデルの不透明度(1で普通、0で見えない)
	float GetOpacity()const { return m_opacity; }
	void SetOpacity(float opacity) { m_opacity = opacity; }
	//死亡演出が終わって消えたことにする
	void Vanish() { m_isVanished = true; }
	//消えたか(消えた敵は更新も描画もしない)
	bool IsVanished()const { return m_isVanished; }
	AttackType GetAttackType()const { return m_attackType; }
	//浮いている高さ(地面から足元まで)
	float GetHoverHeight()const { return m_hoverHeight; }

protected:
	//HPが0以下になったら死亡ステートにする(派生クラスのUpdateの最初に呼ぶ)
	void CheckDeath();

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
	//被弾(のけぞり)のアニメーション(空ならのけぞらない)
	std::string m_damageAnimName;
	//死亡のアニメーション
	std::string m_deathAnimName;
	//叩き落とされて倒れるアニメーション(空なら叩き落とされない)
	std::string m_knockdownAnimName;
	//倒れた状態から起き上がるアニメーション
	std::string m_getUpAnimName;

	//モデルの不透明度(死亡演出で下げていく)
	float m_opacity = 1.0f;
	//死亡演出が終わって消えたか
	bool m_isVanished = false;
	//浮いている高さ
	float m_hoverHeight = 0.0f;
	//攻撃の種類(フレームやダメージはcsvに書く)
	AttackType m_attackType = AttackType::Attack;
};

