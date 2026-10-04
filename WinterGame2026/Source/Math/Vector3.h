#pragma once
#include<DxLib.h>

class Vector3
{
public:
	Vector3();
	Vector3(float initX,float initY,float initZ);
	Vector3(VECTOR vec);
	~Vector3() {};
	float x, y, z;

	//ベクトルの大きさを返す
	float Magnitude()const;
	//ベクトルの大きさの2乗を返します
	float SqMagnitude()const;

	//正規化する
	Vector3 Normalize() const;

	//内積
	float Dot(const Vector3& right)const;

	//外積
	Vector3 Cross(const Vector3& right)const;

	VECTOR ToDxLibVector()const;

	Vector3 RotateY(float angle) const;

	//演算子オーバーロード
	Vector3 operator+(const Vector3& right)const;
	Vector3 operator-(const Vector3& right)const;
	Vector3 operator*(const float& right)const;
	Vector3 operator/(const float& right)const;
	//Vector3 operator=(const float& right)const;
	Vector3 operator+=(const Vector3& right);
	Vector3 operator-=(const Vector3& right);
	Vector3 operator*=(const float& right);
	Vector3 operator/=(const float& right);
	operator VECTOR() const;

};

