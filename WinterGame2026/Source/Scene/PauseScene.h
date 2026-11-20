#pragma once
#include "Scene.h"
#include <vector>

class PauseScene : public Scene
{
public:
	//選んだ時に何をするか
	enum class Action
	{
		//再開する
		Resume,
		Stage1,
		Select,
	};
	//項目
	struct Item
	{
		//表示名
		const char* name;
		//選んだ時に何するか
		Action action;
	};
	/// <summary>
	/// ゲームの上に積んで出すメニューを作る//見出しと項目は積む側が決める
	/// </summary>
	/// <param name="controller">シーンの切り替えを頼む相手</param>
	/// <param name="title">メニューの上に出す見出し</param>
	/// <param name="items">上から順に並べる項目</param>
	/// <param name="firstCursor">最初に選んでおく項目の番号//0が一番上</param>
	PauseScene(SceneController& controller, const char* title,std::vector<Item> items, int firstCursor);
	~PauseScene();

	void Init() override;
	void Update() override;
	void Draw() override;

private:
	//上下の入力から動かす向きを出す
	int GetMoveDir()const;
	//カーソルを動かす//端まで来たら反対側へ回る
	void MoveCursor(int dir);
	//選んでいる項目のActionを行う
	void Decide();

	//見出し
	const char* m_title;
	//並べる項目
	std::vector<Item> m_items;
	//選んでいる項目の番号
	int m_cursor = 0;
	//カーソルの画像
	int m_cursorHandle = -1;
};
