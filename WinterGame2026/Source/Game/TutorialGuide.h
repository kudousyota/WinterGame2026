#pragma once
#include"../System/BattleSystem.h"
#include <vector>

//チュートリアルの達成条件の種類
enum class TutorialGoal
{
	//歩く
	Move,
	//ロックオンする
	LockOn,
	//決めた攻撃を当てた回数
	HitAttack,
	//コンボの何段目まで当てたか
	Combo,
	//叩き落とし
	Knockdown,
	//ジャスト回避
	JustDodge
};

//ステップで数える攻撃
struct TutorialAttack
{
	AttackType type;
	//多段ヒットの何段目が当たったらできたことにするか
	int hitIndex = 1;
};

//ステップで出す敵
struct TutorialEnemy
{
	//SpawnEnemyに渡す名前
	const char* name;
	//地面の位置
	Vector3 groundPos;
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
	//数える攻撃//Comboのときはコンボの順番に並べる
	std::vector<TutorialAttack> attacks;
	//このステップで出す敵//ステップが変わると前の敵は消えて出し直す
	std::vector<TutorialEnemy> enemies;
};

//シーンから渡す、このフレームに起きたこと
struct TutorialInfo
{
	//移動したか
	bool isMoving = false;
	//ロックオンしたか
	bool isLockedOn = false;
	//このフレームに起きた戦闘イベント
	std::vector<BattleEvent> events;
	
};

//今どのステップか何回できたかを数えて達成したら次のステップへ進める
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

