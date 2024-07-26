#pragma once
#include "Vector4F.h"

class Vector2F;
class Vector3;
struct loc;

/*! class for constructing 3D vector*/
class Vector3F
{
public:

	union
	{
		struct{ float x, y, z; }; //for easy access
		struct{ signed a, b, c; }; //for hashing a^b^c
		float vect[3];
	};
	Vector3F(float x = 0, float y = 0, float z = 0);
	Vector3F(const Vector2F& vec, float z = 0);
	~Vector3F();

	Vector3F& operator= (const Vector3& right);
	Vector3F operator+ (const Vector3F& right) const;
	Vector3F operator- (const Vector3F& right) const;
	float dot(const Vector3F& right) const;
	float lengt() const;
	float squareLength() const;
	Vector3F normalize() const;
	Vector3F crossProd(const Vector3F& right) const;
	static Vector4F vec3TOvec4(const Vector3F& vector, float w = 0);
	Vector3 toDouble() const;

	Vector3F operator* (const float& right) const;
	Vector3F operator* (const Vector3F& right) const;
	friend Vector3F operator* (const float& left, const Vector3F& right);
	Vector3F operator/ (const float& right) const;
	Vector3F operator/ (const Vector3F& right) const;

	void operator+=(const Vector3F& v);
	void operator-=(const Vector3F& v);
	void operator*=(const float& number);
	void operator/=(const float& number);
	bool operator==(const Vector3F& v)const;
	float& operator[] (int index);
protected:

};