#pragma once
#include "Vector3.h"

//形状同士の計算をまとめたもの
//キャラクターやステージを知らない純粋な幾何計算だけを置く
namespace Geometry
{
	//2本の線分(p1->q1, p2->q2)の最近接点を求める
	void ClosestPointSegmentSegment(const Vector3& p1, const Vector3& q1, const Vector3& p2, const Vector3& q2, Vector3& outC1, Vector3& outC2);

	//2つのカプセルが重なっているか判定する
	bool CheckCapsule(const Vector3& aStart, const Vector3& aEnd, float radiusA, const Vector3& bStart, const Vector3& bEnd, float radiusB);
}
