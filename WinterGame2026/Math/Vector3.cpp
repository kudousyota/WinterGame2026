#include "Vector3.h"
#include <cmath>
//3D用
Vector3::Vector3():
	x(0.0f),
	y(0.0f),
	z(0.0f)
{
}

Vector3::Vector3(float initX, float initY, float initZ):
	x(initX),
	y(initY),
	z(initZ)
	
{

}

Vector3::Vector3(VECTOR vec):
	x(vec.x),
	y(vec.y),
	z(vec.z)
{
}

float Vector3::Magnitude() const
{
	//三平方の定理の計算
	return sqrtf(this->x * this ->x + this->y * this->y + this->z * this->z);
}

float Vector3::SqMagnitude() const
{
	return this->x * this->x + this->y * this->y + this->z * this->z;
}

Vector3 Vector3::Normalize() const
{
	if (this->Magnitude() <= 0.0f)
	{
		return *this;
	}
	//正規化
	Vector3 ans;
	ans = (*this) / Magnitude();
	return ans;
	
}

float Vector3::Dot(const Vector3& right) const
{
	//内積
	float ans;
	ans = this->x * right.x + this->y * right.y + this->z * right.z;
	return ans;
}

Vector3 Vector3::Cross(const Vector3& right) const
{
	//外積
	Vector3 ans;
	ans.x = this->y * right.z - this->z * right.y;
	ans.y = this->z * right.x - this->x * right.z;
	ans.z = this->x * right.y - this->y * right.x;
	return ans;

}

DxLib::VECTOR Vector3::ToDxLibVector() const
{
	return VGet(this->x, this->y, this->z);
}

Vector3 Vector3::RotateY(float angle) const
{
	float c = cosf(angle);
	float s = sinf(angle);

	return Vector3(
		x * c - z * s,
		y,
		x * s + z * c
	);
}

Vector3 Vector3::operator+(const Vector3& right) const
{
	Vector3 ans;
	ans.x = this->x + right.x;
	ans.y = this->y + right.y;
	ans.z = this->z + right.z;
	return ans;
}

Vector3 Vector3::operator-(const Vector3& right) const
{
	Vector3 ans;
	ans.x = this->x - right.x;
	ans.y = this->y - right.y;
	ans.z = this->z - right.z;
	return ans;

}

Vector3 Vector3::operator*(const float& right) const
{
	Vector3 ans;
	ans.x = this->x * right;
	ans.y = this->y * right;
	ans.z = this->z * right;
	return ans;
}

Vector3 Vector3::operator/(const float& right) const
{
	Vector3 ans;
	ans.x = this->x / right;
	ans.y = this->y / right;
	ans.z = this->z / right;
	return ans;
}

//Vector3 Vector3::operator=(const float& right) const
//{
//	Vector3 ans;
//	ans.x = right;
//	ans.y = right;
//	ans.z = right;
//	return ans;
//}

Vector3 Vector3::operator+=(const Vector3& right)
{
	this->x = this->x + right.x;
	this->y = this->y + right.y;
	this->z = this->z + right.z;
	return *this;
}

Vector3 Vector3::operator-=(const Vector3& right)
{
	this->x = this->x - right.x;
	this->y = this->y - right.y;
	this->z = this->z - right.z;
	return *this;
}

Vector3 Vector3::operator*=(const float& right)
{
	this->x = this->x * right;
	this->y = this->y * right;
	this->z = this->z * right;
	return *this;
}

Vector3 Vector3::operator/=(const float& right)
{
	this->x = this->x / right;
	this->y = this->y / right;
	this->z = this->z / right;
	return *this;
}

Vector3::operator VECTOR() const
{
	return VGet(this->x, this->y, this->z);
}
