#include <cmath>
#include "Vector2.h"
#include "Vector3.h"
namespace mwm
{
Vector2::Vector2(double x, double y) : x(x), y(y) {}

Vector2::Vector2(const Vector2& vec)
{
	x = vec.x;
	y = vec.y;
}

Vector2::~Vector2()
{
}

/*! \fn add vectors amd return new one*/
Vector2 Vector2::operator+ (const Vector2& right) const
{
	return Vector2(x + right.x, y + right.y);
}

/*! \fn substract vectors and return new one*/
Vector2 Vector2::operator- (const Vector2& right) const
{

	return Vector2(x - right.x, y - right.y);
}
/*! \fn dot product returns scalar*/
double Vector2::dotAKAscalar(const Vector2& right) const
{
	return x * right.x + y * right.y;
}

/*! \fn function returning length of instanced vector*/
double Vector2::vectLengt() const
{
	return sqrt(x*x + y*y);
}
/*! \fn function returning new normalized vector*/
Vector2 Vector2::vectNormalize() const
{
	double length = sqrt(x*x + y*y);
	return Vector2(x / length, y / length);
}

/*! \fn function to convert 2D vector to 3D vector*/
Vector3 Vector2::vec2TOvec3(Vector2 vector, double z)
{
	return Vector3(vector.x, vector.y, z);
}

/*! \fn vector*num returns new matrix*/
Vector2 Vector2::operator* (const double& right) const
{
	return Vector2(x * right, y * right);
}

/*! \fn num*vector returns new matrix*/
Vector2 operator* (const double& left, const Vector2& right)
{
	return Vector2(right.x * left, right.y * left);
}

/*! \fn vector/num returns new matrix*/
Vector2 Vector2::operator/ (const double& right) const
{
	return Vector2(x / right, y / right);
}

bool Vector2::operator==(const Vector2& v) const
{
	if (x != v.x || y != v.y) return false;
	else return true;
}
}


