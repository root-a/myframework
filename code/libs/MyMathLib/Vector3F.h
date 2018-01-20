#pragma once
#include <emmintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include "Vector4F.h"
namespace mwm
{
class Vector2F;
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
	Vector3F operator+ (const Vector3F& right) const;
	Vector3F operator- (const Vector3F& right) const;
	float dotAKAscalar(const Vector3F& right) const;
	float vectLengt() const;
	float squareMag() const;
	Vector3F vectNormalize() const;
	Vector3F crossProd(const Vector3F& right) const;
	static Vector4F vec3TOvec4(const Vector3F& vector, float w = 0);

	Vector3F operator* (const float& right) const;
	Vector3F operator* (const Vector3F& right) const;
	friend Vector3F operator* (const float& left, const Vector3F& right);
	Vector3F operator/ (const float& right) const;
	Vector3F operator/ (const Vector3F& right) const;

	Vector3F normalizeSSE() const;
	float vectLengthSSE() const;

	void operator+=(const Vector3F& v);
	void operator-=(const Vector3F& v);
	void operator*=(const float& number);
	void operator/=(const float& number);
	bool operator==(const Vector3F& v)const;
	float& operator[] (int index);
protected:

};

inline Vector3F Vector3F::normalizeSSE() const
{
	__m128 tmp = _mm_set_ps(0, vect[2], vect[1], vect[0]);
	__m128 length = _mm_sqrt_ss(_mm_dp_ps(tmp, tmp, 0x7F));
	__m128 true_length = _mm_shuffle_ps(length, length, _MM_SHUFFLE(0, 0, 0, 0));

	Vector4F normalized;
	normalized.v = _mm_div_ps(tmp, true_length);
	return Vector3F(normalized.x,normalized.y,normalized.z);
}

inline float Vector3F::vectLengthSSE() const
{
	__m128 tmp = _mm_set_ps(0, vect[2], vect[1], vect[0]);
	Vector4F length;
	length.v = _mm_sqrt_ss(_mm_dp_ps(tmp, tmp, 0x7F));
	return length.x;
}
}

