#pragma once
#include <array>
#include <string>
#include <vector>
class Character;
class Camera;

//デバッグ機能(Debugビルドでだけ使う)
//GameSceneが所有する
//
//F1:当たり判定の表示  F2:文字情報の表示  F3:グリッドの表示
//F5:CSVの読み直し
//F6:一時停止  F7:コマ送り(一時停止中)  F8:スロー
class DebugTool
{
public:
	//表示するキャラクター
	struct Target
	{
		const char* name;
		Character* character;
	};

	DebugTool();

	//キー入力を処理する(ゲームの更新より先に毎フレーム呼ぶ)
	void Update(const std::vector<Target>& targets);

	//このフレームにゲームを更新するか(一時停止・スロー中はfalseになる)
	bool ShouldUpdateGame()const { return m_shouldUpdateGame; }

	bool IsShowCollision()const { return m_isShowCollision; }
	bool IsShowGrid()const { return m_isShowGrid; }

	//文字情報を描画する
	void Draw(const std::vector<Target>& targets, const Camera& camera, int frameCount)const;

private:
	//キーが押された瞬間か
	bool IsKeyTriggered(int key)const;
	//CSVを読み直してキャラクターに反映する
	void Reload(const std::vector<Target>& targets);

private:
	//キーボードの状態(今と前のフレーム)
	std::array<char, 256> m_keyState{};
	std::array<char, 256> m_lastKeyState{};

	//パッドのスティックの生の値(-1000～1000)
	int m_leftStickX = 0;
	int m_leftStickY = 0;
	int m_rightStickX = 0;
	int m_rightStickY = 0;

	//表示の切り替え
	bool m_isShowCollision = true;
	bool m_isShowText = true;
	bool m_isShowGrid = true;

	//一時停止中か
	bool m_isPaused = false;
	//スロー中か
	bool m_isSlow = false;
	//スロー中に何フレーム経ったか
	int m_slowCounter = 0;
	//このフレームにゲームを更新するか
	bool m_shouldUpdateGame = true;

	//読み直しの結果を表示する文字と、残りの表示フレーム
	std::string m_message;
	int m_messageFrame = 0;
};
