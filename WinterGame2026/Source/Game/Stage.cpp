#include "Stage.h"
#include "DxLib.h"

namespace
{
	//ステージのモデル
	const char* const kModelPath = "Data/Model/TestStage.mv1";

	//当たり判定用に空間を分割する数(X,Y,Z)
	//細かくするほど判定するポリゴンを絞り込めるが、その分メモリを使う
	constexpr int kCollDivX = 16;
	constexpr int kCollDivY = 4;
	constexpr int kCollDivZ = 16;
}

Stage::Stage()
{
}

Stage::~Stage()
{
	//モデルの解放(当たり判定の情報も一緒に解放される)
	if (m_modelHandle != -1) MV1DeleteModel(m_modelHandle);
}

void Stage::Init()
{
	//ステージは1つしか使わないので、複製せずに直接読み込む
	m_modelHandle = MV1LoadModel(kModelPath);

	//環境光の色をマテリアルの色に合わせる
	//Blenderから書き出したモデルは環境光の色が黒になっていて、ライトが当たらない面が真っ黒になる
	//ライトはカメラの向きに合わせているので、カメラが水平だと床にライトが当たらない
	int materialNum = MV1GetMaterialNum(m_modelHandle);
	for (int i = 0; i < materialNum; i++)
	{
		MV1SetMaterialAmbColor(m_modelHandle, i, MV1GetMaterialDifColor(m_modelHandle, i));
	}

	//当たり判定の情報を作る
	//これを呼んでおかないとMV1CollCheck系の関数が当たらない
	//ステージは動かないので、最初に1回だけ作ればいい
	MV1SetupCollInfo(m_modelHandle, -1, kCollDivX, kCollDivY, kCollDivZ);
}

void Stage::Draw()
{
	MV1DrawModel(m_modelHandle);
}
