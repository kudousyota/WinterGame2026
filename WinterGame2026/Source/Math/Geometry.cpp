#include "Geometry.h"
#include <algorithm>

//線分上で点に一番近い点
Vector3 Geometry::ClosestPointOnSegment(const Vector3& start, const Vector3& end, const Vector3& p)
{
	Vector3 ab = end - start;
	Vector3 ap = p - start;
	//線分の長さの2乗
	float abLen2 = ab.SqMagnitude();
	//線分の長さがほぼ0なら始点がそのまま最近接点
	if (abLen2 <= 0.000001f)
	{
		return start;
	}
	//線分上の最近接点パラメータt(0:始点 1:終点)
	float t = std::clamp(ap.Dot(ab) / abLen2, 0.0f, 1.0f);
	return start + ab * t;
}

//2本の線分
void Geometry::ClosestPointSegmentSegment(const Vector3& p1, const Vector3& q1, const Vector3& p2, const Vector3& q2, Vector3& outC1, Vector3& outC2)
{
	//カプセルAの軸ベクトル
	Vector3 d1 = q1 - p1;
	//カプセルBの軸ベクトル
	Vector3 d2 = q2 - p2;
	//線分１の始点から線分2の始点へのベクトル
	Vector3 r = p1 - p2;
	//線分1の長さの2乗
	float a = d1.Dot(d1);
	//線分2の長さの2乗
	float e = d2.Dot(d2);
	float f = d2.Dot(r);

	float s, t;

	//両方が点(長さ0)の場合
	if (a <= 0.000001f && e <= 0.000001f)
	{
		outC1 = p1;
		outC2 = p2;
		return;
	}

	if (a <= 0.000001f)
	{
		//線分1が点の場合
		s = 0.0f;
		//線分2上の最近接位置を求める
		t = f / e;
		//線分の範囲内に制限
		t = std::clamp(t, 0.0f, 1.0f);
	}
	else
	{
		float c = d1.Dot(r);
		if (e <= 0.000001f)
		{
			//線分2が点の場合
			t = 0.0f;
			//線分1上の最近接位置を求める
			s = std::clamp(-c / a, 0.0f, 1.0f);
		}
		else
		{
			float b = d1.Dot(d2);
			//2本の線分の関係を表す分母
			float denom = a * e - b * b;

			if (denom != 0.0f)
			{
				//線分1側の最近接点を求める
				s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
			}
			else
			{
				//平行な場合
				s = 0.0f;
			}
			//線分2側の最近接点を求める
			t = (b * s + f) / e;
			//線分2の範囲外に出た場合は、
			//線分1側の最近接点を再計算する
			if (t < 0.0f)
			{
				t = 0.0f;
				s = std::clamp(-c / a, 0.0f, 1.0f);
			}
			else if (t > 1.0f)
			{
				t = 1.0f;
				s = std::clamp((b - c) / a, 0.0f, 1.0f);
			}
		}
	}
	//求めたパラメータから実際の最近接点を計算
	outC1 = p1 + d1 * s;
	outC2 = p2 + d2 * t;
}

//2つのカプセルが重なっているか判定する
//カプセル同士の判定は、中心軸となる2本の線分の最短距離と
//2つのカプセル半径の合計を比較して行う
bool Geometry::CheckCapsule(const Capsule& a, const Capsule& b)
{
	//2本の線分同士の最短距離を求める
	float distance = Segment_Segment_MinLength(a.start, a.end, b.start, b.end);

	//両カプセルの半径を合計
	float combinedRadius = a.radius + b.radius;

	// 線分間の距離が半径の合計以下ならカプセル同士が接触している
	return distance <= combinedRadius;
}
