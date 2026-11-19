#pragma once
#include <memory>
#include "../Game/GameManager.h"
#include "../System/DebugTool.h"
#include "Scene.h"
#include "../Game/Character/Enemy/EnemySpawner.h"
#include "../Effect/ScreenDistortion.h"

//本編のシーン
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
	//戦闘で起きたことを受け取って歪みなどの演出を出す
	void HandleBattleEvents();
	//ウェーブを進めて出す敵をGameManagerに渡す
	void UpdateSpawn();
	//勝ち負けが決まっていたら少し待ってからクリアゲームオーバーに切り替える
	void CheckGameEnd();
	//ジャスト回避の歪みを最初から始める
	void StartJustDodgeDistortion();
	//歪みを進めてシェーダーに渡す値を決める
	void UpdateDistortion();

private:
	int m_frameCount;
	//勝ち負けが決まってから経ったフレーム数
	int m_gameEndFrame = 0;

	//本編とチュートリアルで共通のゲームの中身
	GameManager m_game;
	//敵のスポナー//敵を出すタイミングを決める
	EnemySpawner m_spawner;
	//画面の歪み
	ScreenDistortion m_distortion;
	//歪みが始まってから経ったフレーム数
	int m_distortionFrame = -1;
	//ウィッチタイム中の揺らぎの今の強さ
	float m_waveStrength = 0.0f;
	//揺らぎを動かす時間
	float m_waveTime = 0.0f;

#ifdef _DEBUG
	//デバッグ機能
	DebugTool m_debug;
#endif // _DEBUG
};
