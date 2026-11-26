#pragma once

//ゲーム全体で使用する色
//0xRRGGBB で指定する（32bitカラーモード前提。GetColor(r, g, b) と同じ値になる）
namespace Color
{
	//基本色
	constexpr unsigned int kWhite = 0xFFFFFF;
	constexpr unsigned int kBlack = 0x000000;
	constexpr unsigned int kLightGray = 0xAAAAAA;	//(170,170,170)
	constexpr unsigned int kGray = 0x808080;		//(128,128,128)
	constexpr unsigned int kDarkGray = 0x787878;	//(120,120,120)
	constexpr unsigned int kYellow = 0xFFFF00;
	constexpr unsigned int kCyan = 0x00FFFF;
	constexpr unsigned int kGreen = 0x00FF00;

	//ゲームのテーマ色
	constexpr unsigned int kPurple = 0xC878FF;		//(200,120,255) タイトル選択中など
	constexpr unsigned int kGold = 0xFFDC50;		//(255,220,80)  クリアボス名など
	constexpr unsigned int kRed = 0xDC2828;			//(220,40,40)   ゲームオーバー・HP減少
	constexpr unsigned int kWitchTimeOverlay = 0x1C0034;	//(28,0,52) ウィッチタイム中の画面の色
}
