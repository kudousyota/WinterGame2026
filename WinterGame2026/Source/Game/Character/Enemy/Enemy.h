#pragma once
#include "../Character.h"

//敵の基底クラス
//どの敵にも共通する処理だけを持つ
class EnemyManager;
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
	//突進中の走りのアニメーション(設定していなければ移動のアニメーションを使う)
	const std::string& GetChargeRunAnimName()const { return m_chargeRunAnimName.empty() ? m_moveAnimName : m_chargeRunAnimName; }
	//吹っ飛びのアニメーションで、宙に浮いて後ろへ飛んでいる区間の終わり(アニメーション全体に対する割合)
	float GetKnockdownFlyEndRate()const { return m_knockdownFlyEndRate; }
	//吹っ飛びのアニメーションで、地面に倒れきった割合(ここでアニメーションを止める)
	float GetKnockdownLieDownRate()const { return m_knockdownLieDownRate; }

	//死亡ステートから使う
	//モデルの不透明度(1で普通、0で見えない)
	float GetOpacity()const { return m_opacity; }
	void SetOpacity(float opacity) { m_opacity = opacity; }
	//死亡演出が終わって消えたことにする
	void Vanish() { m_isVanished = true; }
	//消えたか(消えた敵は更新も描画もしない)
	bool IsVanished()const { return m_isVanished; }
	AttackType GetAttackType()const { return m_attackType; }
	//次に使う攻撃を決める(攻撃ステートに入る直前に呼ぶ)
	void DecideAttack();
	//この距離から攻撃を始めていいか
	//標準は攻撃範囲の中なら攻撃する。遠くから使える攻撃を持つ敵は上書きする
	virtual bool CanStartAttack(float distance)const;
	//決めた攻撃に合ったステートを作る
	//標準は普通の攻撃ステート。ジャンプ攻撃など動きが違う攻撃を持つ敵は上書きする
	virtual std::unique_ptr<StateBase> CreateAttackState();
	//浮いている高さ(地面から足元まで)
	float GetHoverHeight()const { return m_hoverHeight; }
	//ターゲットから離れる(Y方向の速度は維持)
	void MoveAwayFromTarget(float speedScale);
	//ターゲットの方を向いたまま横に回る(sideは1か-1で回る向き)
	void StrafeAroundTarget(float speedScale, float side);
	//狙う相手
	const Character* GetTarget()const { return m_pTarget; }

	//攻撃判定を出す位置(足元から前へ・上へ)
	float GetHitForwardOffset()const { return m_hitForwardOffset; }
	float GetHitHeight()const { return m_hitHeight; }

	//これより近づかれたら離れる距離
	float GetTooCloseDistance()const { return m_tooCloseDistance; }
	//攻撃の後に離れる距離
	float GetKeepDistance()const { return m_keepDistance; }

	//攻撃の予兆中か(予兆中は光らせる)
	void SetWarning(bool isWarning) { m_isWarning = isWarning; }
	bool IsWarning()const { return m_isWarning; }
	//アニメーションを止める(倒れた姿のまま待つときに使う)
	void SetAnimPause(bool isPause) { m_isAnimPause = isPause; }
	bool IsAnimPause()const { return m_isAnimPause; }

	//敵をまとめて持っているマネージャーを設定する
	void SetManager(EnemyManager* manager) { m_pManager = manager; }
	//攻撃の枠をもらう
	bool TryTakeAttackToken();
	//攻撃の枠を返す
	void ReleaseAttackToken();

#ifdef _DEBUG
	//索敵・攻撃などの範囲の円を描く
	void DrawDebugRange()const;
	//向いている方向(視線)の線を描く
	void DrawDebugForward()const;
	//攻撃の枠を持っているかを頭の上の球で描く
	void DrawDebugToken()const;
	//弱点の当たり判定を描く(弱点のある敵だけ上書きする)
	virtual void DrawDebugWeakPoint()const {}
#endif

protected:
	//HPが0以下になったら死亡ステートにする(派生クラスのUpdateの最初に呼ぶ)
	void CheckDeath();
	//攻撃範囲に入ったときにどの攻撃を使うか
	virtual AttackType SelectAttack();
	//攻撃が終わったときの次のステート
	virtual void OnAttackFinished();
#ifdef _DEBUG
	//範囲や向きを描く中心(プレイヤーの足元の高さに合わせた自分の位置)
	Vector3 GetDebugGroundCenter()const;
#endif

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
	//被弾(のけぞり)のアニメーション(空ならのけぞらない)
	std::string m_damageAnimName;
	//死亡のアニメーション
	std::string m_deathAnimName;
	//叩き落とされて倒れるアニメーション(空なら叩き落とされない)
	std::string m_knockdownAnimName;
	//倒れた状態から起き上がるアニメーション(空なら待機のアニメーションへ切り替えて立つ)
	std::string m_getUpAnimName;
	//突進中の走りのアニメーション(空なら移動のアニメーションを使う)
	std::string m_chargeRunAnimName;
	//吹っ飛びのアニメーションのタイミング(初期値はAlienのFlyingBack・全92フレーム、27フレーム目で着地)
	float m_knockdownFlyEndRate = 0.29f;
	float m_knockdownLieDownRate = 0.9f;

	//モデルの不透明度//色んな使い道ありそう
	float m_opacity = 1.0f;
	//死亡演出が終わって消えたか
	bool m_isVanished = false;
	//浮いている高さ
	float m_hoverHeight = 0.0f;
	//これより近づかれたら離れる距離
	float m_tooCloseDistance = 0.0f;
	//攻撃の後に離れる距離
	float m_keepDistance = 0.0f;
	//攻撃判定を出す位置(足元から前へ・上へ)。体の大きい敵は派生クラスで大きくする
	float m_hitForwardOffset = 50.0f;
	float m_hitHeight = 120.0f;
	//攻撃の予兆中か
	bool m_isWarning = false;
	//アニメーションを止めているか
	bool m_isAnimPause = false;
	//攻撃の種類//フレームやダメージはcsvに書く
	AttackType m_attackType = AttackType::Attack;

	//自分を持っているマネージャー
	EnemyManager* m_pManager = nullptr;
	//攻撃の枠を持っているか
	bool m_hasAttackToken = false;

#ifdef _DEBUG
	//範囲を表示する地面の高さ(プレイヤーが着地している時だけ更新して、ジャンプしても円がついていかないようにする)
	//描画(const)の中で更新するのでmutableにする
	mutable float m_debugGroundY = 0.0f;
	//m_debugGroundYが一度でも決まったか
	mutable bool m_hasDebugGroundY = false;
#endif
};

