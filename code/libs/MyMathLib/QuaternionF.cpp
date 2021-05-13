#include <math.h>
#include "QuaternionF.h"
#include "Vector3F.h"
#include "Matrix4F.h"
#include "Matrix3F.h"
#include "Quaternion.h"
#include "MathUtils.h"
#define _USE_MATH_DEFINES
#include <cmath>

QuaternionF::QuaternionF()
{
	x = 0;
	y = 0;
	z = 0;
	w = 1;
}

QuaternionF::QuaternionF(float angle, const Vector3F &axis)
{
	float PI = 3.14159265f;
	float rAngle = (float)MathUtils::ToRadians(angle);
	x = axis.x * sinf((rAngle) / 2.f);
	y = axis.y * sinf((rAngle) / 2.f);
	z = axis.z * sinf((rAngle) / 2.f);
	w = cosf((rAngle) / 2.f);
}

QuaternionF::QuaternionF(float pitch, float yaw, float roll)
{
	float cy = cosf(yaw * 0.5f);
	float sy = sinf(yaw * 0.5f);
	float cp = cosf(roll * 0.5f);
	float sp = sinf(roll * 0.5f);
	float cr = cosf(pitch * 0.5f);
	float sr = sinf(pitch * 0.5f);
	float c1c2 = cy * cp;
	float s1s2 = sy * sp;
	w = c1c2 * cr - s1s2 * sr;
	x = c1c2 * sr + s1s2 * cr;
	y = sy * cp*cr + cy * sp*sr;
	z = cy * sp*cr - sy * cp*sr;
}

QuaternionF::QuaternionF(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}

QuaternionF::~QuaternionF(void) {}

void QuaternionF::InsertAt(unsigned int index, float value)
{
	quaternion[index] = value;
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

QuaternionF& QuaternionF::operator=(const Quaternion & right)
{
	x = (float)right.x;
	y = (float)right.y;
	z = (float)right.z;
	w = (float)right.w;
	return *this;
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
	return quaternion[index];
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

Vector3F QuaternionF::getLeft() const
{
	float x1 = 1.f - 2.f * y*y - 2.f * z*z;
	float y1 = (2.f * x * y) + (2.f * w * z);
	float z1 = (2.f * x * z) - (2.f * w * y);
	return Vector3F(x1, y1, z1).vectNormalize();
}

Vector3F QuaternionF::getInvLeft() const
{
	float x1 = 1.f - 2.f * y*y - 2.f * z*z;
	float y1 = (2.f * x * y) - (2.f * w * z);
	float z1 = (2.f * x * z) + (2.f * w * y);
	return Vector3F(x1, y1, z1).vectNormalize();
}

Vector3F QuaternionF::getUp() const
{

	float x1 = (2.f * x * y) - (2.f * w * z);
	float y1 = 1.f - 2.f * x*x - 2.f * z*z;
	float z1 = (2.f * y * z) + (2.f * w * x);
	return Vector3F(x1, y1, z1).vectNormalize();
}

Vector3F QuaternionF::getInvUp() const
{
	float x1 = (2.f * x * y) + (2.f * w * z);
	float y1 = 1.f - 2.f * x*x - 2.f * z*z;
	float z1 = (2.f * y * z) - (2.f * w * x);
	return Vector3F(x1, y1, z1).vectNormalize();
}

Vector3F QuaternionF::getBack() const
{
	float x1 = (2.f * x * z) + (2.f * w * y);
	float y1 = (2.f * y * z) - (2.f * w * x);
	float z1 = 1.f - 2.f * x*x - 2.f * y*y;
	return Vector3F(-x1, -y1, -z1).vectNormalize();
}

Vector3F QuaternionF::getInvBack() const
{
	float x1 = (2.f * x * z) - (2.f * w * y);
	float y1 = (2.f * y * z) + (2.f * w * x);
	float z1 = 1.f - 2.f * x*x - 2.f * y*y;
	return Vector3F(-x1, -y1, -z1).vectNormalize();
}

Vector3F QuaternionF::getForward() const
{
	float x1 = (2.f * x * z) + (2.f * w * y);
	float y1 = (2.f * y * z) - (2.f * w * x);
	float z1 = 1.f - 2.f * x*x - 2.f * y*y;
	return Vector3F(x1, y1, z1).vectNormalize();
}

Vector3F QuaternionF::getInvForward() const
{
	float x1 = (2.f * x * z) - (2.f * w * y);
	float y1 = (2.f * y * z) + (2.f * w * x);
	float z1 = 1.f - 2.f * x*x - 2.f * y*y;
	return Vector3F(x1, y1, z1).vectNormalize();
}

Quaternion QuaternionF::toDouble() const
{
	return Quaternion(x, y, z, w);
}

Vector3F QuaternionF::toEulerAngles() const
{
	float yaw = 0.f;
	float pitch = 0.f;
	float roll = 0.f;
	
	float sqw = w*w;
	float sqx = x*x;
	float sqy = y*y;
	float sqz = z*z;
	float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
	float test = x*y + z*w;
	if (test > 0.499f*unit) { // singularity at north pole
		yaw = 2.f * atan2f(x, w);
		pitch = 0.f;
		roll = M_PI / 2.f;
		return Vector3F(pitch, yaw, roll);
	}
	if (test < -0.499f*unit) { // singularity at south pole
		yaw = -2.f * atan2f(x, w);
		pitch = 0.f;
		roll = -M_PI / 2.f;
		return Vector3F(pitch, yaw, roll);
	}
	yaw = atan2f(2.f * y*w - 2.f * x*z, sqx - sqy - sqz + sqw);
	pitch = atan2f(2.f * x*w - 2.f * y*z, -sqx + sqy - sqz + sqw);
	roll = asinf(2.f * test / unit);
	return Vector3F(pitch, yaw, roll);
}