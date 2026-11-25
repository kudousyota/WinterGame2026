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

	//aからbへtの割合だけ進んだ値//tが0ならa、1ならb、0.5ならちょうど真ん中
	inline float Lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}

	//最初は速く最後はゆっくり止まるように0～1の割合を曲げる
	//tは0～1に収めてから渡す
	inline float EaseOutCubic(float t)
	{
		const float rest = 1.0f - t;
		return 1.0f - rest * rest * rest;
	}
}
