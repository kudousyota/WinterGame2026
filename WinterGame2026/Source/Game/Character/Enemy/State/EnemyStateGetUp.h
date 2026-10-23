#pragma once
#include "EnemyStateBase.h"

//敵が倒れた状態から起き上がる
//起き上がりのアニメーションが終わったら待機に戻る(待機のアニメーションへ切り替わるときに浮き上がる)
class EnemyStateGetUp : public EnemyStateBase
{
public:
	using EnemyStateBase::EnemyStateBase;

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::GetUp; }
private:
	//浮き上がっている途中か
	bool m_isRising = false;
	//浮き上がる目標地点
	float m_tagetY = 0.0f;
};
