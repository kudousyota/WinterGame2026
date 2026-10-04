#include "DxLib.h"
#include "Game.h"

#include <memory> //シェアードポイント

#include "Scene/GameScene.h"
#include "System/Application.h"
//定数定義
namespace
{
	//画面サイズ
	constexpr int kScreenWidth = 1280;
	constexpr int kScreenHeight = 720;
}


//プログラムは WinMain から始まります
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	auto& app = Application::GetInstance();
	if (!app.Init())
	{
		return -1;
	}
	app.Run();
	app.Terminate();

	return 0;
}