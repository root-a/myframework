#include <math.h>
#include "Quaternion.h"
#include "Vector3.h"
#include "Matrix4.h"
#include "Matrix3.h"
#include "QuaternionF.h"
#define _USE_MATH_DEFINES
#include <cmath>

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

Quaternion::Quaternion(double pitch, double yaw, double roll)
{
	double cy = cos(yaw * 0.5);
	double sy = sin(yaw * 0.5);
	double cp = cos(roll * 0.5);
	double sp = sin(roll * 0.5);
	double cr = cos(pitch * 0.5);
	double sr = sin(pitch * 0.5);
	double c1c2 = cy * cp;
	double s1s2 = sy * sp;
	w = c1c2 * cr - s1s2 * sr;
	x = c1c2 * sr + s1s2 * cr;
	y = sy * cp*cr + cy * sp*sr;
	z = cy * sp*cr - sy * cp*sr;
}

Quaternion::Quaternion(const double x, const double y, const double z, const double w) : x(x), y(y), z(z), w(w) {}

Quaternion::~Quaternion(void) {}

Quaternion& Quaternion::operator=(const QuaternionF & right)
{
	x = right.x;
	y = right.y;
	z = right.z;
	w = right.w;
	return *this;
}

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

QuaternionF Quaternion::toFloat() const
{
	return QuaternionF((float)x, (float)y, (float)z, (float)w);
}

Vector3 Quaternion::toEulerAngles() const
{
	double yaw = 0.0;
	double pitch = 0.0;
	double roll = 0.0;

	double sqw = w * w;
	double sqx = x * x;
	double sqy = y * y;
	double sqz = z * z;
	double unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
	double test = x * y + z * w;
	if (test > 0.499*unit) { // singularity at north pole
		yaw = 2.0 * atan2(x, w);
		pitch = 0.0;
		roll = M_PI / 2.0;
		return Vector3(pitch, yaw, roll);
	}
	if (test < -0.499*unit) { // singularity at south pole
		yaw = -2.0 * atan2(x, w);
		pitch = 0.0;
		roll = -M_PI / 2.0;
		return Vector3(pitch, yaw, roll);
	}
	yaw = atan2(2.0 * y*w - 2.0 * x*z, sqx - sqy - sqz + sqw);
	pitch = atan2(2.0 * x*w - 2.0 * y*z, -sqx + sqy - sqz + sqw);
	roll = asin(2.0 * test / unit);

	return Vector3(pitch, yaw, roll);
}