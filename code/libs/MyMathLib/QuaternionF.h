#pragma once

class Vector3F;
class Matrix4F;
class Matrix3F;
class Quaternion;

class QuaternionF
{

public:

	union
	{
		float quaternion[4];
		struct{ float x, y, z, w; };
	};

	QuaternionF();
	/// Constructor to set up a QuaternionF
	QuaternionF(float angle, const Vector3F &axis);
	QuaternionF(float yaw, float pitch, float roll);
	QuaternionF(const float x, const float y, const float z, const float w);
	~QuaternionF(void);

	/// Set a value of a component of a vector
	void InsertAt(unsigned int index, float value);

	QuaternionF operator*(const QuaternionF& v) const;
	void operator*=(const QuaternionF& v);
	QuaternionF operator*(const float& number)const;
	QuaternionF& operator= (const Quaternion& right);

	float Magnitude();
	/// Normalize this vector
	void Normalize();
	QuaternionF Normalized() const;
	Matrix4F ConvertToMatrix() const;
	Matrix3F ConvertToMatrix3F() const;

	Vector3F getUp() const;
	Vector3F getInvUp() const;
	Vector3F getLeft() const;
	Vector3F getInvLeft() const;
	Vector3F getBack() const;
	Vector3F getInvBack() const;
	Vector3F getForward() const;
	Vector3F getInvForward() const;
	Quaternion toDouble() const;
	Vector3F toEulerAngles() const;
	/// Overloaded access operator for Vector
	float& operator[](unsigned int index);
};