#pragma once
#include "EnemyStateBase.h"

//敵の死亡状態
//倒れるアニメーション → 少し倒れたまま見せる → だんだん透明にして消す
//消えたら敵のIsVanishedがtrueになる(ゲームクリアの判定に使う)
class EnemyStateDeath : public EnemyStateBase
{
public:
	using EnemyStateBase::EnemyStateBase;

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Death; }
#ifdef _DEBUG
	std::string GetDebugInfo() const override;
#endif

private:
	//倒れるアニメーションが終わってからのフレーム数(ウィッチタイム中はゆっくり進む)
	float m_afterAnimFrame = 0.0f;
};
