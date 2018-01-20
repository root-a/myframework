#pragma once
#include <emmintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>
namespace mwm
{
class Vector2F;
class Vector3F;
/*! class for constructing 4D vector*/
class Vector4F
{
public:
	union
	{
		struct{ float x, y, z, w; };
		float vect[4];
		__m128 v;
	};


	Vector4F(float x = 0, float y = 0, float z = 0, float w = 0);
	Vector4F(const Vector2F& v1, const Vector2F& v2);
	Vector4F(const Vector3F& v1, float w = 0);
	~Vector4F();
	Vector4F operator+ (const Vector4F& right) const;
	Vector4F operator- (const Vector4F& right) const;
	float dotAKAscalar(const Vector4F& right) const;
	float vectLengt() const;
	float squareMag() const;
	Vector4F vectNormalize() const;

	Vector4F operator* (const float& right) const;
	friend Vector4F operator* (const float& left, const Vector4F& right);
	Vector4F operator/ (const float& right) const;

	Vector3F get_xyz() const;
	float& operator[] (int index);

protected:
};
}


