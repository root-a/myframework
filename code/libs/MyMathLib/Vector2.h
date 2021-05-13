#pragma once
class Vector2F;
class Vector3;

class Vector2
{
public:
	union
	{
		struct{ double x, y; };
		struct{ signed a, b; }; //for hashing a^b
		double vect[2];
	};


	Vector2(double x = 0, double y = 0);
	~Vector2();
	Vector2 operator+ (const Vector2& right) const;
	Vector2 operator- (const Vector2& right) const;
	Vector2& operator= (const Vector2F& right);
	double dotAKAscalar(const Vector2& right) const;
	double vectLengt() const;
	Vector2 vectNormalize() const;
	static Vector3 vec2TOvec3(Vector2 vector, double z = 0);

	Vector2 operator* (const double& right) const;
	friend Vector2 operator* (const double& left, const Vector2& right);
	Vector2 operator/ (const double& right) const;
	bool operator==(const Vector2& v)const;
protected:
};