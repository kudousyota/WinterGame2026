#pragma once
#include "Scene.h"
#include "../Game/GameManager.h"
#include "../Game/TutorialGuide.h"
#include "../System/DebugTool.h"

//チュートリアル画面
//ゲームの中身はGameManagerに任せて、ダミーを相手に操作を覚えてもらう
class TutorialScene : public Scene
{
public:
	TutorialScene(SceneController& controller);
	~TutorialScene();

	void Init() override;
	void Update() override;
	void Draw() override;

private:
	//ゲームが進んだフレーム数//デバッグ表示に使う
	int m_frameCount = 0;

	//本編とチュートリアルで共通のゲームの中身
	GameManager m_game;
	//チュートリアルの進行役(今のステップと回数)
	TutorialGuide m_guide;

#ifdef _DEBUG
	//デバッグ機能(一時停止・表示切り替え・CSVの読み直し)
	DebugTool m_debug;
#endif // _DEBUG
};
