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

	//ゲームが進んだフレーム数//デバッグ表示に使う
	int m_frameCount = 0;

	//本編とチュートリアルで共通のゲームの中身
	GameManager m_game;
	//チュートリアルの進行役
	TutorialGuide m_guide;
	//最後に敵を出したステップ//まだ出していなければnullptr
	const TutorialStep* m_spawnedStep = nullptr;

#ifdef _DEBUG
	//デバッグ機能
	DebugTool m_debug;
#endif // _DEBUG
};
