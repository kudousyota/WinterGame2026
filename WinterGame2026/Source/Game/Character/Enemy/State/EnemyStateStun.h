#pragma once
#include "EnemyStateBase.h"

class EnemyStateStun : public EnemyStateBase
{
public:
	//bounceDir:壁から跳ね返る向き
	EnemyStateStun(Enemy& enemy, const Vector3& bounceDir);

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Stun; }
#ifdef _DEBUG
	std::string GetDebugInfo() const override;
#endif

private:
	//跳ね返る向き
	Vector3 m_bounceDir = Vector3(0.0f, 0.0f, 0.0f);
	//スタンしてからのフレーム数
	float m_frame = 0.0f;
	//前のフレームまでに跳ね返った割合
	float m_prevT = 0.0f;
};
