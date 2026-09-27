#pragma once
#include "DxLib.h"

//Model
//ゲーム内で使用する各種3Dモデルのベースを管理するシングルトン。
//起動時にベースモデルを一度読み込み、以後は複製を返すことで複数インスタンスを生成する
//複製数が増えるとメモリを消費するため、必要な時のみ複製を行うこと
class Model
{
public:
	// シングルトン取得
	static Model& Instance()
	{
		static Model instance;
		return instance;
	}

	//起動時に一括でベースモデルをロードする
	void PreloadAll();

	//ベースモデルから複製を作成して返す
	//呼び出し側は複製したモデルの破棄を行う
	int CreatPlayerModel() const { return MV1DuplicateModel(m_playerBaseHandle); }
	//ベースモデルをすべて解放する
	void ReleseAll();

private:
	int m_playerBaseHandle = -1;
};

