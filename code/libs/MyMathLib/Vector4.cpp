#include <cmath>
#include "Vector4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4F.h"

/*! \fn Vector4 constructor*/
Vector4::Vector4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}

Vector4::Vector4(const Vector2& v1, const Vector2& v2)
{
	x = v1.x;
	y = v1.y;
	z = v2.x;
	w = v2.y;
}

Vector4::Vector4(const Vector3& v1, double wIn)
{
	x = v1.x;
	y = v1.y;
	z = v1.z;
	w = wIn;
}

Vector4::~Vector4() {}

Vector4& Vector4::operator=(const Vector4F& right)
{
	x = right.x;
	y = right.y;
	z = right.x;
	w = right.y;
	return *this;
}

/*! \fn add vectors amd return new one*/
Vector4 Vector4::operator+ (const Vector4& right) const
{
	return Vector4(x + right.x, y + right.y, z + right.z, w + right.w);
}

/*! \fn substract vectors and return new one*/
Vector4 Vector4::operator- (const Vector4& right) const
{

	return Vector4(x - right.x, y - right.y, z - right.z, w - right.w);
}

/*! \fn dot product returns scalar*/
double Vector4::dot(const Vector4& right) const
{
	return x * right.x + y * right.y + z * right.z + w * right.w;
}

/*! \fn function returning length of instanced vector*/
double Vector4::lengt() const
{
	return sqrt(x*x + y*y + z*z + w*w);
}

double Vector4::squareLength() const
{
	return x*x + y*y + z*z + w*w;
}

/*! \fn vector*num returns new matrix*/
Vector4 Vector4::operator* (const double& right) const
{
	return Vector4(x * right, y * right, z * right, w * right);
}

/*! \fn num*vector returns new matrix*/
Vector4 operator* (const double& left, const Vector4& right)
{
	return Vector4(right.x * left, right.y * left, right.z * left, right.w * left);
}

/*! \fn vector/num returns new matrix*/
Vector4 Vector4::operator/ (const double& right) const
{
	return Vector4(x / right, y / right, z / right, w / right);
}

Vector3 Vector4::get_xyz() const
{
	return Vector3(x, y, z);
}

/*! \fn function returning new normalized vector*/
Vector4 Vector4::normalize() const
{
	double squareLength = x * x + y * y + z * z + w * w;
	if (squareLength == 0.0) return Vector4();
	double length = sqrt(squareLength);
	return Vector4(x / length, y / length, z / length, w / length);
}

/*! \fn operator[] overload for indexing */
double& Vector4::operator[] (int index)
{
	return vect[index];
}