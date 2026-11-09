#pragma once
#include "EnemyStateBase.h"
#include "../../../../DataLoader/AttackTable.h"

class EnemyStateCharge : public EnemyStateBase
{
public:
	explicit EnemyStateCharge(Enemy& enemy);
	void Enter() override;
	void Update()override;
	void Exit() override;
	StateKind GetKind()const override { return StateKind::Attack; }

private:
	void StopTelegraph();

	AttackTable::AttackInfo m_info;
	float m_frame;
	int m_attackId;
	int m_telegraphHandle = -1;
	Vector3 m_charageDir = Vector3(0.0f, 0.0f, 0.0f);
	bool m_hasStarted = false;
};

