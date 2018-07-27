#include <cmath>
#include "Vector2F.h"
#include "Vector3F.h"
namespace mwm
{
Vector2F::Vector2F(float x, float y) : x(x), y(y) {}

Vector2F::Vector2F(const Vector2F& vec)
{
	x = vec.x;
	y = vec.y;
}

Vector2F::~Vector2F() {}

/*! \fn add vectors amd return new one*/
Vector2F Vector2F::operator+ (const Vector2F& right) const
{
	return Vector2F(x + right.x, y + right.y);
}

/*! \fn substract vectors and return new one*/
Vector2F Vector2F::operator- (const Vector2F& right) const
{
	return Vector2F(x - right.x, y - right.y);
}
/*! \fn dot product returns scalar*/
float Vector2F::dotAKAscalar(const Vector2F& right) const
{
	return x * right.x + y * right.y;
}

/*! \fn function returning length of instanced vector*/
float Vector2F::vectLengt() const
{
	return sqrt(x*x + y*y);
}
/*! \fn function returning new normalized vector*/
Vector2F Vector2F::vectNormalize() const
{
	float length = sqrtf(x*x + y*y);
	return Vector2F(x / length, y / length);
}

/*! \fn function to convert 2D vector to 3D vector*/
Vector3F Vector2F::vec2TOvec3(Vector2F vector, float z)
{
	return Vector3F(vector.x, vector.y, z);
}

/*! \fn vector*num returns new matrix*/
Vector2F Vector2F::operator* (const float& right) const
{
	return Vector2F(x * right, y * right);
}

/*! \fn num*vector returns new matrix*/
Vector2F operator* (const float& left, const Vector2F& right)
{
	return Vector2F(right.x * left, right.y * left);
}

/*! \fn vector/num returns new matrix*/
Vector2F Vector2F::operator/ (const float& right) const
{
	return Vector2F(x / right, y / right);
}

bool Vector2F::operator==(const Vector2F& v)const
{
	if (x != v.x || y != v.y) return false;
	else return true;
}
}


