#include "Application.h"
#include "DxLib.h"
#include <cassert>
#include "../System/Input.h"
//#include "../Scene/SceneController.h"
//#include "../Scene/TitleScene.h"
#include "../Scene/GameScene.h"
#include "../Game.h"
//#include "EffekseerForDXLib.h"
//#include "Model.h"
//#include "Timer.h"
//#include "../Effect/EffectManager.h"
//#include "SoundManager.h"
//#include "../Common/FontManager.h"


Application::Application()
{

}

Application::~Application()
{
}

Application& Application::GetInstance()
{
	//staticなのでメモリの場所が一つで確定
	static Application instance;
	return instance;

}

bool Application::Init()
{

	//ウィンドウモード設定
	ChangeWindowMode(false);
	//タイトル変更
	SetMainWindowText("Kudonetta");
	//画面のサイズ変更
	SetGraphMode(Game::kScreenWidth, Game::kScreenHeight, Game::kColorBitNum);
	//フォントの読み込み
	//AddFontResourceEx("Constantia", FR_PRIVATE, NULL);

	//DirectX11をしようするための初期化
	//Effecseerを使用するには必ず設定する
	SetUseDirect3DVersion(DX_DIRECT3D_11);

	if (DxLib_Init() == -1)
	{
		return false;
	}

	//Effekseer使用の初期化
	//引数には画面に表示する最大パーティクルを設定する
	//if (Effekseer_Init(8000) == -1)
	//{
	//	return -1;			//エラーが起きたら直ちに終了
	//}

	//フルスクリーンウインドウの切り替えでリソースが消えるのを防ぐ
	SetChangeScreenModeGraphicsSystemResetFlag(false);

	//描画対象をバックバッファに変更
	SetDrawScreen(DX_SCREEN_BACK);
	//カリングの設定
	SetUseBackCulling(true);
	//モデルのロード
	//Model::Instance().PreloadAll();
	//サウンドのロード
	//SoundManager::Instance().Load();
	//フォントのロード
	//FontManager::Instance().LoadFont();
	//背景色を設定(ここでRGBを変更すれば背景色を変えれる)
	//薄い青
	//SetBackgroundColor(100, 150, 255);
	//黒
	SetBackgroundColor(0, 0, 0);

	return true;
}

void Application::Run()
{
	Input::Instance();
	//SceneController controller;
	//controller.ChangeScene(std::make_shared<TitleScene>(controller));

	//一旦SceneMainを表示する
	GameScene sceneMain;
	sceneMain.Init();


	while (ProcessMessage() != -1 && !m_IsRequstendExit)
	{

		//このフレームの開始時間を取得
		LONGLONG start = GetNowHiPerformanceCount();

		ClearDrawScreen();
		Input::Instance().Update();
		//タイマーの更新(時間スケールやウィッチタイムの残りフレームを減らす)
		//Timer::Instance().Update();

		//controller.Update(input);
		//controller.Draw();

		if (CheckHitKey(KEY_INPUT_ESCAPE))
		{
			break;
		}
		ScreenFlip();

		//フレームレート６０に固定
		while (GetNowHiPerformanceCount() - start < 16667)
		{

		}
	}
}

void Application::Terminate()
{
	//フォントの解放
	//RemoveFontResourceEx("Constantia", FR_PRIVATE, NULL);
	//FontManager::Instance().Destroy();
	//EffectManagerの終了処理を先に行う
	//EffectManager::Instns().Terminate();
	//Effekseer 使用の終了処理
	//Effkseer_End();
	//プリロードしたモデルを解放
	//Model::Instance().ReleseAll();
	DxLib_End();
}

void Application::RequestExit()
{
	m_IsRequstendExit = true;
}
