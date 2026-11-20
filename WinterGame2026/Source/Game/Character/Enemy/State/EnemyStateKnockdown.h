#pragma once
#include "EnemyStateBase.h"

class EnemyStateKnockdown : public EnemyStateBase
{
public:
	//distance:吹っ飛ぶ距離(AttackHit.csvのknockback)
	EnemyStateKnockdown(Enemy& enemy, float distance);

	void Enter() override;
	void Update() override;
	void Exit() override;
	StateKind GetKind() const override { return StateKind::Down; }
#ifdef _DEBUG
	std::string GetDebugInfo() const override;
#endif

private:
	//吹っ飛ぶ方向
	Vector3 m_flyDir = Vector3(0.0f,0.0f,0.0f);
	//吹っ飛ぶ距離
	float m_distance = 0.0f;
	//前のフレームまでに進んだ割合
	float m_prevT = 0.0f;
	//倒れきってからのフレーム数
	float m_lieFrame = 0.0f;
	//地面まで落ちる高さ(叩き落とされた瞬間の高さ)
	float m_fallHeight = 0.0f;
};
