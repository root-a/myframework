#include <cmath>
#include "Vector4.h"
#include "Vector2.h"
#include "Vector3.h"
namespace mwm
{
/*! \fn Vector4 constructor*/
Vector4::Vector4(double x, double y, double z, double w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

Vector4::Vector4(const Vector2& v1, const Vector2& v2)
{
	this->x = v1.x;
	this->y = v1.y;
	this->z = v2.x;
	this->w = v2.y;
}

Vector4::Vector4(const Vector3& v1, double w)
{
	this->x = v1.x;
	this->y = v1.y;
	this->z = v1.z;
	this->w = w;
}

Vector4::~Vector4()
{
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
double Vector4::dotAKAscalar(const Vector4& right) const
{
	return x * right.x + y * right.y + z * right.z + w * right.w;
}

/*! \fn function returning length of instanced vector*/
double Vector4::vectLengt() const
{
	return sqrt(x*x + y*y + z*z + w*w);
}

double Vector4::squareMag() const
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
Vector4 Vector4::vectNormalize() const
{
	double length = sqrt(x*x + y*y + z*z + w*w);
	return Vector4(x / length, y / length, z / length, w / length);
}

/*! \fn operator[] overload for indexing */
double& Vector4::operator[] (int index)
{
	return this->vect[index];
}
}