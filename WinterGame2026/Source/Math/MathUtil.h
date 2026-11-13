#pragma once
#include "DxLib.h"

//ゲームのあちこちで使う計算
namespace MathUtil
{
	//角度(ラジアン)を-π～πに直す
	//目標の角度との差に使うと、遠回りせずに近い方へ回れる
	//回り続けて何周分も大きくなった角度でも直せるようにwhileで繰り返す
	inline float WrapAngle(float angle)
	{
		while (angle > DX_PI_F)  angle -= DX_TWO_PI_F;
		while (angle < -DX_PI_F) angle += DX_TWO_PI_F;
		return angle;
	}
}
