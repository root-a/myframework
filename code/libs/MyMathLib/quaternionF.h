#pragma once
namespace mwm
{
class Vector3F;
class Matrix4F;
class Matrix3F;

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
	QuaternionF(const float x, const float y, const float z, const float w);
	~QuaternionF(void);

	/// Set a value of a component of a vector
	void InsertAt(unsigned int index, float value);

	QuaternionF operator*(const QuaternionF& v) const;
	void operator*=(const QuaternionF& v);
	QuaternionF operator*(const float& number)const;

	float Magnitude();
	/// Normalize this vector
	void Normalize();
	QuaternionF Normalized() const;
	Matrix4F ConvertToMatrix() const;
	Matrix3F ConvertToMatrix3F() const;

	/// Overloaded access operator for Vector
	float& operator[](unsigned int index);
};
}

