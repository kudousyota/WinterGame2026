#pragma once
#include <array>
#include <initializer_list>
#include <string>
#include <vector>
class Character;
class Camera;

//デバッグ機能(Debugビルドでだけ使う)
//GameSceneが所有する
//
//TAB:表示メニューを開く/閉じる(開いている間はゲームが止まる)
//      ↑↓で選んで、Enter(または←→)で表示を切り替える
//F1:当たり判定(体とボスの弱点と攻撃)の表示  F2:文字情報の表示  F3:グリッドの表示  F4:ロックオン範囲の表示
//F5:CSVの読み直し
//F6:一時停止  F7:コマ送り(一時停止中)  F8:スロー
//F9:敵のデバッグ表示(範囲・向き・攻撃枠)をまとめて切り替え
//F11:プレイヤーの足元の位置を、Spawn.csvの1行としてクリップボードにコピー
//K:いま出ている敵を全部倒す(ウェーブを飛ばしてボスまで進むときに使う)
class DebugTool
{
public:
	//表示するキャラクター
	struct Target
	{
		//敵は"Alien0"のように番号を付けて作るので、文字列で持つ
		std::string name;
		Character* character;
	};

	//表示を切り替えられるデバッグ描画の種類
	//メニューにはこの順番で並ぶ
	enum class Show
	{
		Text,			//文字情報
		Grid,			//グリッド
		Collision,		//当たり判定(体・ジャスト回避・ボスの弱点)
		AttackHit,		//攻撃判定(このフレームの攻撃・ブレス)
		LockOn,			//ロックオンできる範囲
		EnemyRange,		//敵の索敵・攻撃などの範囲の円
		EnemyForward,	//敵の向いている方向の線
		EnemyToken,		//敵の攻撃枠のマーク
		Num
	};

	DebugTool();

	//キー入力を処理する(ゲームの更新より先に毎フレーム呼ぶ)
	void Update(const std::vector<Target>& targets);

	//このフレームにゲームを更新するか(一時停止・スロー中はfalseになる)
	bool ShouldUpdateGame()const { return m_shouldUpdateGame; }

	//その種類のデバッグ描画を表示するか
	bool IsShow(Show type)const { return m_isShow[static_cast<int>(type)]; }

	//文字情報と表示メニューを描画する
	void Draw(const std::vector<Target>& targets, const Camera& camera, int frameCount)const;

private:
	//キーが押された瞬間か
	bool IsKeyTriggered(int key)const;
	//表示の切り替え
	void Toggle(Show type);
	//まとめて切り替える(1つ目がオンなら全部オフ、それ以外は全部オン)
	void ToggleGroup(std::initializer_list<Show> types);
	//表示メニューの操作
	void UpdateMenu();
	//文字情報を描画する
	void DrawInfoText(const std::vector<Target>& targets, const Camera& camera, int frameCount)const;
	//表示メニューを描画する
	void DrawMenu()const;
	//CSVを読み直してキャラクターに反映する
	void Reload(const std::vector<Target>& targets);
	//プレイヤーの足元の位置をSpawn.csvの1行の形にしてクリップボードにコピーする
	void CopySpawnLine(const std::vector<Target>& targets);
	//いま出ている敵を全部倒す(HPを0にして、いつもどおり死亡演出をさせる)
	void KillEnemies(const std::vector<Target>& targets);

private:
	//キーボードの状態(今と前のフレーム)
	std::array<char, 256> m_keyState{};
	std::array<char, 256> m_lastKeyState{};

	//パッドのスティックの生の値(-1000～1000)
	int m_leftStickX = 0;
	int m_leftStickY = 0;
	int m_rightStickX = 0;
	int m_rightStickY = 0;

	//種類ごとの表示の切り替え(最初は全部表示)
	std::array<bool, static_cast<int>(Show::Num)> m_isShow{};

	//表示メニューを開いているか
	bool m_isMenuOpen = false;
	//メニューで選んでいる項目
	int m_menuCursor = 0;

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
