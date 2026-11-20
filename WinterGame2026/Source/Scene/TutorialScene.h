#pragma once
#include "Scene.h"
#include "../Game/GameManager.h"
#include "../Game/TutorialGuide.h"
#include "../System/DebugTool.h"

class TutorialScene : public Scene
{
public:
	TutorialScene(SceneController& controller);
	~TutorialScene();

	void Init() override;
	void Update() override;
	void Draw() override;

private:
	//ステップが変わっていたら、前の敵を消してそのステップの敵を出す
	void UpdateStepEnemies();
	//全部終わって少し経ったら、完了メニューを一度だけ出す
	void UpdateFinishMenu();
	//ポーズメニューを積む//ステージ1へは全部終わってから選べる
	void OpenPauseMenu();

	//ゲームが進んだフレーム数//デバッグ表示に使う
	int m_frameCount = 0;

	//本編とチュートリアルで共通のゲームの中身
	GameManager m_game;
	//チュートリアルの進行役
	TutorialGuide m_guide;
	//最後に敵を出したステップ//まだ出していなければnullptr
	const TutorialStep* m_spawnedStep = nullptr;
	//全部終わってから完了メニューを出すまで数える
	int m_finishFrame = 0;
	//完了メニューを出したか//出したあとは自由に練習する
	bool m_isFreePractice = false;

#ifdef _DEBUG
	//デバッグ機能
	DebugTool m_debug;
#endif // _DEBUG
};
