#pragma once
class Vector2;
class Vector3;
class Vector4F;

class Vector4
{
public:
	union
	{
		struct{ double x, y, z, w; };
		double vect[4];
	};


	Vector4(double x = 0, double y = 0, double z = 0, double w = 0);
	Vector4(const Vector2& v1, const Vector2& v2);
	Vector4(const Vector3& v1, double w = 0);
	~Vector4();
	Vector4& operator= (const Vector4F& right);
	Vector4 operator+ (const Vector4& right) const;
	Vector4 operator- (const Vector4& right) const;
	double dotAKAscalar(const Vector4& right) const;
	double vectLengt() const;
	double squareMag() const;
	Vector4 vectNormalize() const;

	Vector4 operator* (const double& right) const;
	friend Vector4 operator* (const double& left, const Vector4& right);
	Vector4 operator/ (const double& right) const;

	Vector3 get_xyz() const;
	double& operator[] (int index);

protected:
};