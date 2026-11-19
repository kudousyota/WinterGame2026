#include "TutorialGuide.h"
#include <iterator>
namespace
{
	//上から順にやる
	//ステップの一覧
	const TutorialStep kSteps[] =
	{
		{"スティックで移動してみよう",TutorialGoal::Move,120},
		{"LBで敵をロックオン！",TutorialGoal::LockOn,1},
	};
	//ステップの数//要素を順番にたどる
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
	}
}

void TutorialGuide::NextStep()
{
	m_stepIndex++;
	m_count = 0;
}
