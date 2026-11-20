#include "TutorialGuide.h"
#include <iterator>
#include <algorithm>
namespace
{
	//敵を出す位置//部屋の真ん中から少し奥
	const Vector3 kEnemyPos = Vector3(0.0f, 0.0f, 600.0f);

	//上から順にやる//ステップの一覧
	//攻撃を数えないステップも、敵を書くときは間に{}をはさむ
	const TutorialStep kSteps[] =
	{
		{"スティックで移動してみよう",TutorialGoal::Move,120,{},{{"Dummy",kEnemyPos}}},
		{"LBで敵をロックオン！",TutorialGoal::LockOn,1,{},{{"Dummy",kEnemyPos}}},
		{"Xで3連続攻撃を当てよう",TutorialGoal::Combo,3,{{AttackType::Punch},{AttackType::Punching},{AttackType::Rush,4}},{{"Dummy",kEnemyPos}}},
		{"Yで敵にキックしてみよう",TutorialGoal::HitAttack,3,{{AttackType::Kick},{AttackType::MmaKick},{AttackType::SideKick}},{{"Dummy",kEnemyPos}}},
		{"AでジャンプしてYで敵を叩き落としてみよう", TutorialGoal::Knockdown, 1,{},{{"DummyFloat",kEnemyPos}}},
		{"敵の攻撃をBで回避",TutorialGoal::JustDodge,1,{},{{"DummyAttack",kEnemyPos}}}
	};
	//ステップの数
	constexpr int kStepNum = static_cast<int>(std::size(kSteps));

	//達成してから次のステップへ進むまでのフレーム数//できたか見せる時間
	constexpr int kClearWaitFrame = 120;
}


void TutorialGuide::Init(int startStep)
{
	m_stepIndex = startStep;
	m_count = 0;
	m_clearWaitFrame = 0;
}

void TutorialGuide::Update(const TutorialInfo& info)
{
	const TutorialStep* step = GetCurrentStep();

	if (step == nullptr)return;
	
	if (IsCleared())
	{
		m_clearWaitFrame--;
		if (m_clearWaitFrame <= 0)
		{
			NextStep();
		}
		return;
	}
	CountUp(*step, info);

	//必要な回数に届いたら達成
	if (m_count >= step->requiredCount)
	{
		m_clearWaitFrame = kClearWaitFrame;
	}
}

const TutorialStep* TutorialGuide::GetCurrentStep() const
{
	if (m_stepIndex < 0 || m_stepIndex >= kStepNum)
	{
		return nullptr;
	}

	return &kSteps[m_stepIndex];
}

int TutorialGuide::GetCount() const
{
	return m_count;
}

bool TutorialGuide::IsCleared() const
{
	return m_clearWaitFrame > 0;
}

bool TutorialGuide::IsFinished() const
{
	return GetCurrentStep() == nullptr;
}

void TutorialGuide::CountUp(const TutorialStep& step, const TutorialInfo& info)
{
	switch (step.goal)
	{
	case TutorialGoal::Move:
		if (info.isMoving)m_count++;
		break;
	case TutorialGoal::LockOn:
		if (info.isLockedOn)m_count++;
		break;
	case TutorialGoal::HitAttack:
		for (const BattleEvent& event : info.events)
		{
			//当たったイベント以外は数えない
			if (event.type != BattleEvent::Type::Hit)continue;
			//無敵などで当たっていないものも数えない
			if (event.result == HitResult::Ignored)continue;
			//このステップで数える攻撃なら
			for (const TutorialAttack& attack : step.attacks)
			{
				if (event.attackType == attack.type)
				{
					m_count++;
					break;
				}
			}
		}
		break;
	case TutorialGoal::Combo:
		for (const BattleEvent& event : info.events)
		{
			if (event.type != BattleEvent::Type::Hit)continue;
			if (event.result == HitResult::Ignored)continue;

			for (int i = 0; i < static_cast<int>(step.attacks.size()); i++)
			{
				//違う攻撃なら次を見る
				if (event.attackType != step.attacks[i].type)continue;
				//まだ決めた段目に届いていなければ数えない
				if (event.hitIndex < step.attacks[i].hitIndex)continue;
				//その段までできた//途中からやり直しても減らないように大きいときだけ入れる
				m_count = (std::max)(m_count, i + 1);
			}
		}
		break;
	case TutorialGoal::JustDodge:
		for (const BattleEvent& event : info.events)
		{
			if (event.type == BattleEvent::Type::JustDodge)m_count++;
		}
		break;
	case TutorialGoal::Knockdown:
		for (const BattleEvent& event : info.events)
		{
			if (event.type != BattleEvent::Type::Hit)continue;
			//叩き落としたら数える
			if (event.result == HitResult::Knockdown)m_count++;
		}
		break;
	}
}

void TutorialGuide::NextStep()
{
	m_stepIndex++;
	m_count = 0;
}
