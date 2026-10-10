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
	int m_modelHandle = -1;
};
