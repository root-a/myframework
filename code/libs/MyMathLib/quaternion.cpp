#include <math.h>
#include "quaternion.h"
#include "Vector3.h"
#include "Matrix4.h"
#include "Matrix3.h"

namespace mwm
{
Quaternion::Quaternion()
{
	x = 0;
	y = 0;
	z = 0;
	w = 1;
}

Quaternion::Quaternion(double angle, const Vector3 &axis)
{
	x = axis.x * sin((angle) / 2.0);
	y = axis.y * sin((angle) / 2.0);
	z = axis.z * sin((angle) / 2.0);
	w = cos((angle) / 2.0);
}

Quaternion::Quaternion(const double x, const double y, const double z, const double w) : x(x), y(y), z(z), w(w)
{}

Quaternion::~Quaternion(void)
{
}

void Quaternion::InsertAt(unsigned int index, double value)
{
	this->quaternion[index] = value;
}

Quaternion Quaternion::operator*(const Quaternion& v) const
{
	Quaternion result;

	result.x = w*v.x + x*v.w + y*v.z - z*v.y;
	result.y = w*v.y - x*v.z + y*v.w + z*v.x;
	result.z = w*v.z + x*v.y - y*v.x + z*v.w;
	result.w = w*v.w - x*v.x - y*v.y - z*v.z;

	return result;
}

Quaternion Quaternion::operator*(const double& number) const
{
	return Quaternion(x * number, y * number, z * number, w * number);
}

void Quaternion::operator*=(const Quaternion& v)
{
	x = w*v.x + x*v.w + y*v.z - z*v.y;
	y = w*v.y - x*v.z + y*v.w + z*v.x;
	z = w*v.z + x*v.y - y*v.x + z*v.w;
	w = w*v.w - x*v.x - y*v.y - z*v.z;
}

double Quaternion::Magnitude()
{
	return sqrt(x*x + y*y + z*z + w*w);
}

void Quaternion::Normalize()
{
	double length = sqrt(x*x + y*y + z*z + w*w);
	x /= length;
	y /= length;
	z /= length;
	w /= length;
}

Quaternion Quaternion::Normalized() const
{
	double length = sqrt(x*x + y*y + z*z + w*w);
	return Quaternion(x / length, y / length, z / length, w / length);
}

Matrix4 Quaternion::ConvertToMatrix() const
{
	Matrix4 rotation;
	rotation[0][0] = 1.0 - 2.0 * y*y - 2.0 * z*z;
	rotation[1][0] = (2.0 * x * y) - (2.0 * w * z);
	rotation[2][0] = (2.0 * x * z) + (2.0 * w * y);
	rotation[3][0] = 0.0;

	rotation[0][1] = (2.0 * x * y) + (2.0 * w * z);
	rotation[1][1] = 1.0 - 2.0 * x*x - 2.0 * z*z;
	rotation[2][1] = (2.0 * y * z) - (2.0 * w * x);
	rotation[3][1] = 0.0;

	rotation[0][2] = (2.0 * x * z) - (2.0 * w * y);
	rotation[1][2] = (2.0 * y * z) + (2.0 * w * x);
	rotation[2][2] = 1.0 - 2.0 * x*x - 2.0 * y*y;
	rotation[3][2] = 0.0;

	rotation[0][3] = 0.0;
	rotation[1][3] = 0.0;
	rotation[2][3] = 0.0;
	rotation[3][3] = 1.0;

	return rotation;
}

double& Quaternion::operator[](unsigned int index)
{
	return this->quaternion[index];
}

Matrix3 Quaternion::ConvertToMatrix3() const
{
	Matrix3 rotation;
	rotation[0][0] = 1.0 - 2.0 * y*y - 2.0 * z*z;
	rotation[1][0] = (2.0 * x * y) - (2.0 * w * z);
	rotation[2][0] = (2.0 * x * z) + (2.0 * w * y);

	rotation[0][1] = (2.0 * x * y) + (2.0 * w * z);
	rotation[1][1] = 1.0 - 2.0 * x*x - 2.0 * z*z;
	rotation[2][1] = (2.0 * y * z) - (2.0 * w * x);

	rotation[0][2] = (2.0 * x * z) - (2.0 * w * y);
	rotation[1][2] = (2.0 * y * z) + (2.0 * w * x);
	rotation[2][2] = 1.0 - 2.0 * x*x - 2.0 * y*y;

	return rotation;
}

}
