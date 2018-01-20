#include <math.h>
#include "quaternionF.h"
#include "Vector3F.h"
#include "Matrix4F.h"
#include "Matrix3F.h"

namespace mwm
{
QuaternionF::QuaternionF()
{
	this->x = 0;
	this->y = 0;
	this->z = 0;
	this->w = 1;
}

QuaternionF::QuaternionF(float angle, const Vector3F &axis)
{
	this->x = axis.x * sinf((angle) / 2.f);
	this->y = axis.y * sinf((angle) / 2.f);
	this->z = axis.z * sinf((angle) / 2.f);
	this->w = cosf((angle) / 2.f);
}

QuaternionF::QuaternionF(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w)
{}

QuaternionF::~QuaternionF(void)
{
}

void QuaternionF::InsertAt(unsigned int index, float value)
{
	this->quaternion[index] = value;
}

QuaternionF QuaternionF::operator*(const QuaternionF& v) const
{
	QuaternionF result;

	result.x = w*v.x + x*v.w + y*v.z - z*v.y;
	result.y = w*v.y - x*v.z + y*v.w + z*v.x;
	result.z = w*v.z + x*v.y - y*v.x + z*v.w;
	result.w = w*v.w - x*v.x - y*v.y - z*v.z;

	return result;
}

QuaternionF QuaternionF::operator*(const float& number) const
{
	return QuaternionF(x * number, y * number, z * number, w * number);
}

void QuaternionF::operator*=(const QuaternionF& v)
{
	x = w*v.x + x*v.w + y*v.z - z*v.y;
	y = w*v.y - x*v.z + y*v.w + z*v.x;
	z = w*v.z + x*v.y - y*v.x + z*v.w;
	w = w*v.w - x*v.x - y*v.y - z*v.z;
}

float QuaternionF::Magnitude()
{
	return sqrtf(x*x + y*y + z*z + w*w);
}

void QuaternionF::Normalize()
{
	float length = sqrtf(x*x + y*y + z*z + w*w);
	x /= length;
	y /= length;
	z /= length;
	w /= length;
}

QuaternionF QuaternionF::Normalized() const
{
	float length = sqrtf(x*x + y*y + z*z + w*w);
	return QuaternionF(x / length, y / length, z / length, w / length);
}

Matrix4F QuaternionF::ConvertToMatrix() const
{
	Matrix4F rotation;
	rotation[0][0] = 1.f - 2.f * y*y - 2.f * z*z;
	rotation[1][0] = (2.f * x * y) - (2.f * w * z);
	rotation[2][0] = (2.f * x * z) + (2.f * w * y);
	rotation[3][0] = 0.f;

	rotation[0][1] = (2.f * x * y) + (2.f * w * z);
	rotation[1][1] = 1.f - 2.f * x*x - 2.f * z*z;
	rotation[2][1] = (2.f * y * z) - (2.f * w * x);
	rotation[3][1] = 0.f;

	rotation[0][2] = (2.f * x * z) - (2.f * w * y);
	rotation[1][2] = (2.f * y * z) + (2.f * w * x);
	rotation[2][2] = 1.f - 2.f * x*x - 2.f * y*y;
	rotation[3][2] = 0.f;

	rotation[0][3] = 0.f;
	rotation[1][3] = 0.f;
	rotation[2][3] = 0.f;
	rotation[3][3] = 1.f;

	return rotation;
}

float& QuaternionF::operator[](unsigned int index)
{
	return this->quaternion[index];
}

Matrix3F QuaternionF::ConvertToMatrix3F() const
{
	Matrix3F rotation;
	rotation[0][0] = 1.f - 2.f * y*y - 2.f * z*z;
	rotation[1][0] = (2.f * x * y) - (2.f * w * z);
	rotation[2][0] = (2.f * x * z) + (2.f * w * y);

	rotation[0][1] = (2.f * x * y) + (2.f * w * z);
	rotation[1][1] = 1.f - 2.f * x*x - 2.f * z*z;
	rotation[2][1] = (2.f * y * z) - (2.f * w * x);

	rotation[0][2] = (2.f * x * z) - (2.f * w * y);
	rotation[1][2] = (2.f * y * z) + (2.f * w * x);
	rotation[2][2] = 1.f - 2.f * x*x - 2.f * y*y;

	return rotation;
}

}
