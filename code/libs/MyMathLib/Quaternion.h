#pragma once
namespace mwm
{
class Vector3;
class Matrix4;
class Matrix3;

class Quaternion
{

public:

	union
	{
		double quaternion[4];
		struct{ double x, y, z, w; };
	};

	Quaternion();
	/// Constructor to set up a quaternion
	Quaternion(double angle, const Vector3 &axis);
	Quaternion(const double x, const double y, const double z, const double w);
	~Quaternion(void);

	/// Set a value of a component of a vector
	void InsertAt(unsigned int index, double value);

	Quaternion operator*(const Quaternion& v) const;
	void operator*=(const Quaternion& v);
	Quaternion operator*(const double& number)const;

	double Magnitude();
	/// Normalize this vector
	void Normalize();
	Quaternion Normalized() const;
	Matrix4 ConvertToMatrix() const;
	Matrix3 ConvertToMatrix3() const;

	Vector3 getUp() const;
	Vector3 getInvUp() const;
	Vector3 getLeft() const;
	Vector3 getInvLeft() const;
	Vector3 getBack() const;
	Vector3 getInvBack() const;
	Vector3 getForward() const;
	Vector3 getInvForward() const;

	/// Overloaded access operator for Vector
	double& operator[](unsigned int index);
};
}

