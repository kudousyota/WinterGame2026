#pragma once
#include "Vector3.h"

//形状同士の計算をまとめたもの
//キャラクターやステージを知らない純粋な幾何計算だけを置く
namespace Geometry
{
	//カプセル(軸の線分start->endと半径)
	struct Capsule
	{
		Vector3 start;
		Vector3 end;
		float radius = 0.0f;
	};

	//線分(start->end)上で点pに一番近い点を求める
	Vector3 ClosestPointOnSegment(const Vector3& start, const Vector3& end, const Vector3& p);

	//2本の線分(p1->q1, p2->q2)の最近接点を求める
	void ClosestPointSegmentSegment(const Vector3& p1, const Vector3& q1, const Vector3& p2, const Vector3& q2, Vector3& outC1, Vector3& outC2);

	//2つのカプセルが重なっているか判定する
	bool CheckCapsule(const Capsule& a, const Capsule& b);
}
