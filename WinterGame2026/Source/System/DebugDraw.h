#pragma once
#include "../Math/Vector3.h"

//デバッグ用の図形を描く関数(範囲の確認などに使う)
namespace DebugDraw
{
	//centerからdirを中心に左右halfAngleの扇形を水平に描く(innerが0なら内側の弧は描かない)
	void DrawFan(const Vector3& center, const Vector3& dir, float inner, float outer, float halfAngle, unsigned int color);
	//centerを中心にした水平な円を描く
	void DrawCircle(const Vector3& center, float radius, unsigned int color);
}
