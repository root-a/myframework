#pragma once

class Vector3;
class Matrix4;
class Matrix3;
class QuaternionF;

class Quaternion
{

public:

	union
	{
		double quaternion[4];
		struct{ double x, y, z, w; };
	};
	//quaternion multiplication x * y * z
	//matrix multiplication z * y * x
	Quaternion();
	/// Constructor to set up a quaternion
	Quaternion(double angle, const Vector3 &axis);
	Quaternion toAngleAxis();
	Quaternion(double pitch, double yaw, double roll);
	Quaternion(const Vector3& pitchYawRoll);
	Quaternion(const double x, const double y, const double z, const double w);
	~Quaternion(void);
	Quaternion& operator= (const QuaternionF& right);
	/// Set a value of a component of a vector
	void insertAt(unsigned int index, double value);

	Quaternion operator*(const Quaternion& v) const;
	void operator*=(const Quaternion& v);
	Quaternion operator*(const double& number)const;

	double length();
	/// normalize this vector
	void normalize();
	Quaternion normalized() const;
	Matrix4 convertToMatrix() const;
	Matrix3 convertToMatrix3() const;

	Vector3 getUp() const;
	Vector3 getInvUp() const;
	Vector3 getLeft() const;
	Vector3 getInvLeft() const;
	Vector3 getBack() const;
	Vector3 getInvBack() const;
	Vector3 getForward() const;
	Vector3 getInvForward() const;
	QuaternionF toFloat() const;
	Vector3 toEulerAngles() const;

	/// Overloaded access operator for Vector
	double& operator[](unsigned int index);
};