#include <cmath>
#include "Vector2F.h"
#include "Vector3F.h"
#include "Vector2.h"
#include <cstring>

Vector2F::Vector2F(float x, float y) : x(x), y(y) {}

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

Vector2F& Vector2F::operator=(const Vector2& right)
{
	x = (float)right.x;
	y = (float)right.y;
	return *this;
}
/*! \fn dot product returns scalar*/
float Vector2F::dot(const Vector2F& right) const
{
	return x * right.x + y * right.y;
}

/*! \fn function returning length of instanced vector*/
float Vector2F::lengt() const
{
	return sqrt(x*x + y*y);
}
/*! \fn function returning new normalized vector*/
Vector2F Vector2F::normalize() const
{
	float length = sqrtf(x*x + y*y);
	return Vector2F(x / length, y / length);
}

Vector2 Vector2F::toDouble() const
{
	return Vector2(x, y);
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
	return !std::memcmp((void*)this, (void*)&v, sizeof(Vector2F));
	//if (x != v.x || y != v.y) return false;
	//else return true;
}