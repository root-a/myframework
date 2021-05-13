#include <cmath>
#include "Vector4F.h"
#include "Vector2F.h"
#include "Vector3F.h"
#include "Vector4.h"

/*! \fn Vector4F constructor*/
Vector4F::Vector4F(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

Vector4F::Vector4F(const Vector2F& v1, const Vector2F& v2)
{
	x = v1.x;
	y = v1.y;
	z = v2.x;
	w = v2.y;
}

Vector4F::Vector4F(const Vector3F& v1, float wIn)
{
	x = v1.x;
	y = v1.y;
	z = v1.z;
	w = wIn;
}

Vector4F::~Vector4F()
{
}

Vector4F& Vector4F::operator=(const Vector4& right)
{
	x = (float)right.x;
	y = (float)right.y;
	z = (float)right.x;
	w = (float)right.y;
	return *this;
}

/*! \fn add vectors amd return new one*/
Vector4F Vector4F::operator+ (const Vector4F& right) const
{
	return Vector4F(x + right.x, y + right.y, z + right.z, w + right.w);
}

/*! \fn substract vectors and return new one*/
Vector4F Vector4F::operator- (const Vector4F& right) const
{
	return Vector4F(x - right.x, y - right.y, z - right.z, w - right.w);
}

/*! \fn dot product returns scalar*/
float Vector4F::dotAKAscalar(const Vector4F& right) const
{
	return x * right.x + y * right.y + z * right.z + w * right.w;
}

/*! \fn function returning length of instanced vector*/
float Vector4F::vectLengt() const
{
	return sqrtf(x*x + y*y + z*z + w*w);
}

float Vector4F::squareMag() const
{
	return x*x + y*y + z*z + w*w;
}

/*! \fn vector*num returns new matrix*/
Vector4F Vector4F::operator* (const float& right) const
{
	return Vector4F(x * right, y * right, z * right, w * right);
}

/*! \fn num*vector returns new matrix*/
Vector4F operator* (const float& left, const Vector4F& right)
{
	return Vector4F(right.x * left, right.y * left, right.z * left, right.w * left);
}

/*! \fn vector/num returns new matrix*/
Vector4F Vector4F::operator/ (const float& right) const
{
	return Vector4F(x / right, y / right, z / right, w / right);
}

Vector3F Vector4F::get_xyz() const
{
	return Vector3F(x, y, z);
}

/*! \fn function returning new normalized vector*/
Vector4F Vector4F::vectNormalize() const
{
	float squareMag = x * x + y * y + z * z + w * w;
	if (squareMag == 0.0) return Vector4F();
	float length = sqrt(squareMag);
	return Vector4F(x / length, y / length, z / length, w / length);
}

Vector4 Vector4F::toDouble() const
{
	return Vector4(x, y, z, w);
}

/*! \fn operator[] overload for indexing */
float& Vector4F::operator[] (int index)
{
	return vect[index];
}