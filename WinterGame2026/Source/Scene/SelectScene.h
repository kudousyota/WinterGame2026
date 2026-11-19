#pragma once
#include "Scene.h"

//タイトルの次の画面
//チュートリアル・ステージを選んで始める
class SelectScene : public Scene
{
public:
	//選べる項目
	//戻ってきたときにカーソルを合わせるなど、外から項目を指定できるようにpublicに置く
	//表示名や説明などの中身はSelectScene.cppに書く
	enum class Item
	{
		Tutorial,
		Stage1,
		Stage2,
		Num,
	};

	SelectScene(SceneController& controller);

	void Init() override;
	void Update() override;
	void Draw() override;

private:
	//上下の入力から動かす向きを出す
	int GetMoveDir()const;
	//カーソルを動かす//端まで来たら反対側へ回る
	void MoveCursor(int dir);
	//選んでいる項目で始める//ロック中なら揺らすだけ
	void Decide();

	//選んでいる項目の番号
	int m_cursor = 0;
	//ロック中の項目を決定したときに揺らす残りフレーム数
	int m_shakeFrame = 0;
};
