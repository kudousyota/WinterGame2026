#include "DebugDraw.h"
#include "DxLib.h"

namespace
{
	//弧を何本の線で描くか
	constexpr int kDiv = 32;
}

void DebugDraw::DrawFan(const Vector3& center, const Vector3& dir, float inner, float outer, float halfAngle, unsigned int color)
{
	auto point = [&](float angle, float radius) { return center + dir.RotateY(angle) * radius; };

	//弧
	for (int i = 0; i < kDiv; i++)
	{
		float a0 = -halfAngle + 2.0f * halfAngle * i / kDiv;
		float a1 = -halfAngle + 2.0f * halfAngle * (i + 1) / kDiv;
		DrawLine3D(point(a0, outer), point(a1, outer), color);
		if (inner > 0.0f) DrawLine3D(point(a0, inner), point(a1, inner), color);
	}
	//左右の端(円のときは描かない)
	if (halfAngle < DX_PI_F)
	{
		DrawLine3D(point(-halfAngle, inner), point(-halfAngle, outer), color);
		DrawLine3D(point(halfAngle, inner), point(halfAngle, outer), color);
	}
}

void DebugDraw::DrawCircle(const Vector3& center, float radius, unsigned int color)
{
	DrawFan(center, Vector3(0.0f, 0.0f, 1.0f), radius, radius, DX_PI_F, color);
}
