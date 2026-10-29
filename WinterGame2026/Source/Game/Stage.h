#pragma once

//ステージ
//ステージのモデルを読み込み、描画と当たり判定用のハンドルを提供する
class Stage
{
public:
	Stage();
	~Stage();

	void Init();
	void Draw();

	//当たり判定に使うモデルのハンドル
	int GetModelHandle()const { return m_modelHandle; }

private:
	//空(画面全体のグラデーション)を描く
	void DrawSky() const;

	//当たり判定と描画に使うモデル(地面・建物)
	int m_modelHandle = -1;
	//遠景のモデル(描画だけ・当たり判定なし)
	int m_bgHandle = -1;
};
