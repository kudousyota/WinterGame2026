#include "ScreenDistortion.h"
#include "DxLib.h"
#include "../Game.h"
#include <array>
#include <cassert>

namespace
{
	//DxLibが使っていない定数バッファの番号
	constexpr int kConstantBufferSlot = 4;
}

ScreenDistortion::~ScreenDistortion()
{
	Terminate();
}

void ScreenDistortion::Init()
{
	m_screen = MakeScreen(Game::kScreenWidth, Game::kScreenHeight, false);
	m_copyScreen = MakeScreen(Game::kScreenWidth, Game::kScreenHeight, false);
	m_pixelShader = LoadPixelShader("Data/Shader/DistortionPS.pso");
	assert(m_pixelShader != -1 && "歪みシェーダーが読めない(hlslのビルド設定を確認)");
	m_gradeShader = LoadPixelShader("Data/Shader/WitchGradePS.pso");
	assert(m_gradeShader != -1 && "ウィッチタイムの色のシェーダーが読めない(hlslのビルド設定を確認)");
	m_constantBuffer = CreateShaderConstantBuffer(sizeof(Param));
	m_param.aspect = static_cast<float>(Game::kScreenWidth) / static_cast<float>(Game::kScreenHeight);
}

void ScreenDistortion::Terminate()
{
	if (m_screen != -1) DeleteGraph(m_screen);
	if (m_copyScreen != -1) DeleteGraph(m_copyScreen);
	if (m_pixelShader != -1) DeleteShader(m_pixelShader);
	if (m_gradeShader != -1) DeleteShader(m_gradeShader);
	if (m_constantBuffer != -1) DeleteShaderConstantBuffer(m_constantBuffer);
	m_screen = -1;
	m_copyScreen = -1;
	m_pixelShader = -1;
	m_gradeShader = -1;
	m_constantBuffer = -1;
}

void ScreenDistortion::BeginCapture()
{
	SetDrawScreen(m_screen);
	ClearDrawScreen();
}

void ScreenDistortion::ApplyGrade()
{
	//ウィッチタイムでないときやシェーダーが無いときは何もしない
	if (m_param.gradeAmount <= 0.0f || m_gradeShader == -1) return;

	//描画先はm_screenのまま、今の中身を写してから色を変えて上書きする
	//2Dの描画はZバッファに書かないので、ステージの奥行きは残り、この後のキャラクターの前後関係は崩れない
	GetDrawScreenGraph(0, 0, Game::kScreenWidth, Game::kScreenHeight, m_copyScreen);
	DrawWithShader(m_gradeShader, m_copyScreen);
}

void ScreenDistortion::EndCapture()
{
	SetDrawScreen(DX_SCREEN_BACK);

	//輪も揺らぎも無いときや、シェーダーが無いときはそのまま貼る
	if ((m_param.strength <= 0.0f && m_param.waveStrength <= 0.0f) || m_pixelShader == -1)
	{
		DrawGraph(0, 0, m_screen, false);
		return;
	}
	DrawWithShader(m_pixelShader, m_screen);
}

void ScreenDistortion::DrawWithShader(int pixelShader, int texture)
{
	//定数バッファに今の設定を書き込む
	Param* buffer = static_cast<Param*>(GetBufferShaderConstantBuffer(m_constantBuffer));
	*buffer = m_param;
	//画面サイズで決まるので毎回入れ直す
	buffer->aspect = static_cast<float>(Game::kScreenWidth) / static_cast<float>(Game::kScreenHeight);
	UpdateShaderConstantBuffer(m_constantBuffer);
	SetShaderConstantBuffer(m_constantBuffer, DX_SHADERTYPE_PIXEL, kConstantBufferSlot);

	//全画面の四角形(Zの字の順に並べる)
	const float w = static_cast<float>(Game::kScreenWidth);
	const float h = static_cast<float>(Game::kScreenHeight);
	std::array<VERTEX2DSHADER, 4> vertices = {};
	const float pos[4][2] = { {0.0f, 0.0f}, {w, 0.0f}, {0.0f, h}, {w, h} };
	const float uv[4][2] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f} };
	for (int i = 0; i < 4; i++)
	{
		vertices[i].pos = VGet(pos[i][0], pos[i][1], 0.0f);
		vertices[i].rhw = 1.0f;
		vertices[i].dif = GetColorU8(255, 255, 255, 255);
		vertices[i].spc = GetColorU8(0, 0, 0, 0);
		vertices[i].u = uv[i][0];
		vertices[i].v = uv[i][1];
		vertices[i].su = 0.0f;
		vertices[i].sv = 0.0f;
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	SetUsePixelShader(pixelShader);
	SetUseTextureToShader(0, texture);
	DrawPrimitive2DToShader(vertices.data(), static_cast<int>(vertices.size()), DX_PRIMTYPE_TRIANGLESTRIP);
	//次の描画に影響しないように戻す
	SetUseTextureToShader(0, -1);
	SetUsePixelShader(-1);
}
