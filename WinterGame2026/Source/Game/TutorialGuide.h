#pragma once

//チュートリアルの達成条件の種類
enum class TutorialGoal
{
	//歩く
	Move,
	//ロックオンする
	LockOn
};

//チュートリアルの1ステップ分のデータ
struct TutorialStep
{
	//画面下に出す課題
	const char* text;
	//何をしたら数えるか
	TutorialGoal goal;
	//何回で達成か
	int requiredCount;
};

//シーンから渡す、このフレームに起きたこと
struct TutorialInfo
{
	//移動したか
	bool isMoving = false;
	//ロックオンしたか
	bool isLockedOn = false;
};

//チュートリアルの進行役
//今どのステップか・何回できたかを数えて、達成したら次のステップへ進める
class TutorialGuide
{
public:
	void Init(int startStep);
	void Update(const TutorialInfo& info);

	//今のステップ//全部終わったらnullptr
	const TutorialStep* GetCurrentStep()const;

	int GetCount()const;

	bool IsCleared()const;

	bool IsFinished()const;

private:
	//今のステップの条件に合っていたら数える
	void CountUp(const TutorialStep& step, const TutorialInfo& info);
	//次のステップへ進む
	void NextStep();

	//今のステップの番号
	int m_stepIndex = 0;
	//今のステップで数えた回数
	int m_count = 0;
	//達成してから次へ進むまでの残りフレーム数
	int m_clearWaitFrame = 0;

};

