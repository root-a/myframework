#pragma once

class Vector4;
class Vector3F;
class Vector2;
struct loc;

class Vector3
{
public:

	union
	{
		struct{ double x, y, z; }; //for easy access
		struct{ signed a, b, c; }; //for hashing a^b^c
		double vect[3];
	};
	Vector3(double x = 0, double y = 0, double z = 0);
	Vector3(const Vector2& vec, double z = 0);
	Vector3(const Vector4& vec);
	~Vector3();
	Vector3& operator= (const Vector3F& right);
	Vector3& operator= (const Vector4& right);
	Vector3 operator+ (const Vector3& right) const;
	Vector3 operator- (const Vector3& right) const;
	double dot(const Vector3& right) const;
	double lengt() const;
	double squareLength() const;
	Vector3 normalize() const;
	Vector3 crossProd(const Vector3& right) const;
	static Vector4 vec3TOvec4(const Vector3& vector, double w = 0);
	Vector3F toFloat() const;
	Vector3 operator* (const double& right) const;
	Vector3 operator* (const Vector3& right) const;
	friend Vector3 operator* (const double& left, const Vector3& right);
	Vector3 operator/ (const double& right) const;
	Vector3 operator/ (const Vector3& right) const;
	void zero();
	void one();
	
	void operator+=(const Vector3& v);
	void operator-=(const Vector3& v);
	void operator*=(const double& number);
	void operator/=(const double& number);
	bool operator==(const Vector3& v)const;
	double& operator[] (int index);
protected:

};