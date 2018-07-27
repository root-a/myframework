#include <math.h>
#include "Quaternion.h"
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
	double PI = 3.14159265;
	double dAngle = angle * PI / 180.0;
	x = axis.x * sin((dAngle) / 2.0);
	y = axis.y * sin((dAngle) / 2.0);
	z = axis.z * sin((dAngle) / 2.0);
	w = cos((dAngle) / 2.0);
}

Quaternion::Quaternion(const double x, const double y, const double z, const double w) : x(x), y(y), z(z), w(w) {}

Quaternion::~Quaternion(void) {}

void Quaternion::InsertAt(unsigned int index, double value)
{
	quaternion[index] = value;
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
	return quaternion[index];
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

Vector3 Quaternion::getLeft() const
{
	double x1 = 1.0 - 2.0 * y*y - 2.0 * z*z;
	double y1 = (2.0 * x * y) + (2.0 * w * z);
	double z1 = (2.0 * x * z) - (2.0 * w * y);
	return Vector3(x1, y1, z1).vectNormalize();
}

Vector3 Quaternion::getInvLeft() const
{
	double x1 = 1.0 - 2.0 * y*y - 2.0 * z*z;
	double y1 = (2.0 * x * y) - (2.0 * w * z);
	double z1 = (2.0 * x * z) + (2.0 * w * y);
	return Vector3(x1, y1, z1).vectNormalize();
}

Vector3 Quaternion::getUp() const
{
	
	double x1 = (2.0 * x * y) - (2.0 * w * z);
	double y1 = 1.0 - 2.0 * x*x - 2.0 * z*z;
	double z1 = (2.0 * y * z) + (2.0 * w * x);
	return Vector3(x1, y1, z1).vectNormalize();
}

Vector3 Quaternion::getInvUp() const
{
	double x1 = (2.0 * x * y) + (2.0 * w * z);
	double y1 = 1.0 - 2.0 * x*x - 2.0 * z*z;
	double z1 = (2.0 * y * z) - (2.0 * w * x);
	return Vector3(x1, y1, z1).vectNormalize();
}

Vector3 Quaternion::getBack() const
{
	double x1 = (2.0 * x * z) + (2.0 * w * y);
	double y1 = (2.0 * y * z) - (2.0 * w * x);
	double z1 = 1.0 - 2.0 * x*x - 2.0 * y*y;
	return Vector3(-x1, -y1, -z1).vectNormalize();
}

Vector3 Quaternion::getInvBack() const
{
	double x1 = (2.0 * x * z) - (2.0 * w * y);
	double y1 = (2.0 * y * z) + (2.0 * w * x);
	double z1 = 1.0 - 2.0 * x*x - 2.0 * y*y;
	return Vector3(-x1, -y1, -z1).vectNormalize();
}

Vector3 Quaternion::getForward() const
{
	double x1 = (2.0 * x * z) + (2.0 * w * y);
	double y1 = (2.0 * y * z) - (2.0 * w * x);
	double z1 = 1.0 - 2.0 * x*x - 2.0 * y*y;
	return Vector3(x1, y1, z1).vectNormalize();
}

Vector3 Quaternion::getInvForward() const
{
	double x1 = (2.0 * x * z) - (2.0 * w * y);
	double y1 = (2.0 * y * z) + (2.0 * w * x);
	double z1 = 1.0 - 2.0 * x*x - 2.0 * y*y;
	return Vector3(x1, y1, z1).vectNormalize();
}

}
