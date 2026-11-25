#pragma once
#include <memory>
#include "../Game/GameManager.h"
#include "../System/DebugTool.h"
#include "Scene.h"
#include "../Game/Character/Enemy/EnemySpawner.h"
#include "../UI/UIManager.h"

class GameScene : public Scene
{
public:
	GameScene(SceneController& controller);
	~GameScene();

	void Init() override;
	void Update() override;
	void Draw() override;
private:
	void DrawGrid();
	//戦闘で起きたことを受け取る//歪みとカメラの揺れはGameManagerが出す
	void HandleBattleEvents();
	//ウェーブを進めて出す敵をGameManagerに渡す
	void UpdateSpawn();
	//勝ち負けが決まっていたら少し待ってからクリアゲームオーバーに切り替える
	void CheckGameEnd();
	//ポーズメニューを積む
	void OpenPauseMenu();

private:
	int m_frameCount;
	//勝ち負けが決まってから経ったフレーム数
	int m_gameEndFrame = 0;

	//本編とチュートリアルで共通のゲームの中身
	GameManager m_game;
	//敵のスポナー//敵を出すタイミングを決める
	EnemySpawner m_spawner;
	//画面に出すUIをまとめて持つ
	UIManager m_uiManager;

#ifdef _DEBUG
	//デバッグ機能
	DebugTool m_debug;
#endif // _DEBUG
};
