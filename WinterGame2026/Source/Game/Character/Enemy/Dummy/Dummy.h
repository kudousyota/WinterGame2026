#pragma once
#include "../Enemy.h"

class Dummy : public Enemy
{
public:
	//ダミーの動き方
	enum class Mode
	{
		//棒立ち//攻撃の練習用
		Stand,
		//その場で一定の間隔で攻撃してくる//回避とジャスト回避の練習用
		Attack,
		//浮いている//叩き落としの練習用
		Float,
	};

	explicit Dummy(Mode mode);
	~Dummy()override;
	void Init()override;
	void Update()override;
	void LateUpdate()override;
	void Draw()override;

	//HPが0になりそうなら最大まで戻して死なないようにする
	void Damage(int damage)override;
	//吹っ飛ぶ攻撃を受けたら止まったあとに消えて戻ってくるようにする
	HitResult OnHit(const AttackData& attackData, int damage)override;

	//Attackモードのときだけ攻撃する
	bool CanStartAttack(float distance)const override;
	//攻撃のあとは離れずにその場で待つ
	std::unique_ptr<StateBase> CreateAfterAttackState()override;

private:
	//消えて戻ってくる流れ
	enum class RespawnPhase
	{
		//普通に動いている
		None,
		//吹っ飛んで止まったので、薄くなって消えている
		FadeOut,
		//最初の場所に戻って、濃くなって出てきている
		FadeIn,
	};

	//吹っ飛ばされてもう動きが止まったか
	bool IsBlownAwayFinished()const;
	//消えて戻ってくる流れを進める//ステートを止めておくならtrueを返す
	bool UpdateRespawn(float timeScale);
	//最初に出た場所に戻して待機から始め直す
	void Respawn();

	Mode m_mode;
	int m_modelHandle = -1;

	//最初に出た場所//吹っ飛ばされたらここに戻る
	Vector3 m_homePos;
	//m_homePosを覚えたか//出す位置はInitの後にSpawnEnemyで決まるので、最初の更新で覚える
	bool m_hasHomePos = false;
	//吹っ飛ぶ攻撃を受けて、まだ戻っていないか
	bool m_isBlownAway = false;
	RespawnPhase m_respawnPhase = RespawnPhase::None;
	//今の段階になってから経ったフレーム数
	float m_respawnFrame = 0.0f;
};
