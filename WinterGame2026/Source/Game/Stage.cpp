#include "Stage.h"
#include "DxLib.h"

namespace
{
	//ステージのモデル(描画用)
	const char* const kModelPath = "Data/Model/TempleStage.mv1";
	//ステージの当たり判定用モデル(描画しない)
	const char* const kCollModelPath = "Data/Model/TempleStage_Coll.mv1";
	//遠景のモデル(浮島・雲海・空の光輪など。遠くまであるので当たり判定には入れない)
	const char* const kBgModelPath = "Data/Model/GothicStage_BG.mv1";

	//遠景を描くときだけ使う、映る一番遠い距離
	//遠景は400m先まであるので、通常のカメラの距離では切れてしまう
	constexpr float kBgFar = 60000.0f;

	//空の色(上・地平線・下)
	//Blenderの空の色はmv1に入らないので、画面に直接グラデーションを描く
	constexpr unsigned char kSkyTop[3] = { 40, 50, 110 };
	constexpr unsigned char kSkyHorizon[3] = { 245, 175, 110 };
	constexpr unsigned char kSkyBottom[3] = { 255, 120, 60 };
	//地平線の色にする高さ(画面の上からの割合)
	constexpr float kSkyHorizonRate = 0.55f;

	//当たり判定用に空間を分割する数(X,Y,Z)
	//細かくするほど判定するポリゴンを絞り込めるが、その分メモリを使う
	//ステージは奥(Z)に長い(幅約50m・奥行約190m)ので、Zを細かく分ける
	constexpr int kCollDivX = 8;
	constexpr int kCollDivY = 4;
	constexpr int kCollDivZ = 64;
}

Stage::Stage()
{
}

Stage::~Stage()
{
	//モデルの解放(当たり判定の情報も一緒に解放される)
	if (m_modelHandle != -1) MV1DeleteModel(m_modelHandle);
	if (m_collHandle != -1) MV1DeleteModel(m_collHandle);
	if (m_bgHandle != -1) MV1DeleteModel(m_bgHandle);
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

	//当たり判定の情報は、当たり判定用のモデルにだけ作る
	//これを呼んでおかないとMV1CollCheck系の関数が当たらない
	//ステージは動かないので、最初に1回だけ作ればいい
	m_collHandle = MV1LoadModel(kCollModelPath);
	MV1SetupCollInfo(m_collHandle, -1, kCollDivX, kCollDivY, kCollDivZ);

	//遠景は見た目だけなので当たり判定は作らない
	m_bgHandle = MV1LoadModel(kBgModelPath);
	if (m_bgHandle != -1)
	{
		int bgMaterialNum = MV1GetMaterialNum(m_bgHandle);
		for (int i = 0; i < bgMaterialNum; i++)
		{
			MV1SetMaterialAmbColor(m_bgHandle, i, MV1GetMaterialDifColor(m_bgHandle, i));
		}
	}
}

void Stage::Draw()
{
	//空は全部の後ろなので一番最初に描く
	DrawSky();

	//遠景は空の次に奥にあるので、空の後に描く
	//映る距離を一時的に伸ばして描き、元に戻してから奥行きの情報を消す
	//(消さないと、距離の設定が違う遠景の奥行きでステージやキャラクターが隠れてしまう)
	if (m_bgHandle != -1)
	{
		float cameraNear = GetCameraNear();
		float cameraFar = GetCameraFar();
		SetCameraNearFar(cameraNear, kBgFar);
		MV1DrawModel(m_bgHandle);
		SetCameraNearFar(cameraNear, cameraFar);
		ClearDrawScreenZBuffer();
	}

	MV1DrawModel(m_modelHandle);
}

void Stage::DrawSky() const
{
	int width = 0;
	int height = 0;
	GetDrawScreenSize(&width, &height);
	float horizonY = height * kSkyHorizonRate;

	//上・地平線・下の3段の頂点を左右に置き、間の色は自動で混ざる
	const float ys[3] = { 0.0f, horizonY, static_cast<float>(height) };
	const unsigned char* colors[3] = { kSkyTop, kSkyHorizon, kSkyBottom };

	VERTEX2D vertex[6] = {};
	for (int i = 0; i < 6; i++)
	{
		int row = i / 2;
		vertex[i].pos = VGet((i % 2 == 0) ? 0.0f : static_cast<float>(width), ys[row], 0.0f);
		vertex[i].rhw = 1.0f;
		vertex[i].dif = GetColorU8(colors[row][0], colors[row][1], colors[row][2], 255);
	}
	DrawPrimitive2D(vertex, 6, DX_PRIMTYPE_TRIANGLESTRIP, DX_NONE_GRAPH, false);
}
