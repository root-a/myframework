#pragma once
class Vector3F;
class Vector2;

class Vector2F
{
public:
	union
	{
		struct{ float x, y; };
		struct{ signed a, b; }; //for hashing a^b
		float vect[2];
	};


	Vector2F(float x = 0, float y = 0);
	~Vector2F();
	Vector2F operator+ (const Vector2F& right) const;
	Vector2F operator- (const Vector2F& right) const;
	Vector2F& operator= (const Vector2& right);
	float dot(const Vector2F& right) const;
	float lengt() const;
	Vector2F normalize() const;
	Vector2 toDouble() const;
	static Vector3F vec2TOvec3(Vector2F vector, float z = 0);

	Vector2F operator* (const float& right) const;
	friend Vector2F operator* (const float& left, const Vector2F& right);
	Vector2F operator/ (const float& right) const;
	bool operator==(const Vector2F& v)const;
protected:
};