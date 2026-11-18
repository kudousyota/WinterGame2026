#pragma once
#include "../Enemy.h"

//チュートリアルで使う死なない練習相手
//見た目と動きはゴブリンを使い回す
//段階が変わるときは消して別のモードで出し直す
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

	//Attackモードのときだけ攻撃する
	bool CanStartAttack(float distance)const override;
	//攻撃のあとは離れずにその場で待つ
	std::unique_ptr<StateBase> CreateAfterAttackState()override;

private:
	Mode m_mode;
	int m_modelHandle = -1;
};
