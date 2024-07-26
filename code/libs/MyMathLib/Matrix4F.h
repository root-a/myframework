#pragma once
#include "Vector4F.h"

class Vector3F;
class Matrix3F;
class Matrix4;
class QuaternionF;

struct loc;

class Matrix4F
{
public:
	union
	{
		float _matrix[4][4];
		float _matrix16[16];
		struct
		{
			Vector4F right, up, dir, position;
		} v;
		Vector4F component[4];
	};

	Matrix4F();
	Matrix4F(int identity);
	~Matrix4F();

	Matrix4F operator~ (); //!< transpose matrix returns new matrix
	Matrix4F& operator= (const Matrix4& rightMatrix); //!< copy matrix returns this matrix
	Matrix4 toDouble() const;//!< copy matrix returns this matrix
	bool operator== (const Matrix4F& rightMatrix) const; //!< check if matrices are identical
	Matrix4F operator* (const Matrix4F& rightMatrix) const; //!< matrix*matrix returns new matrix
	float operator() (int row, int col); //!< operator() overload for indexing
	Matrix4F operator* (const float& rightFloat) const; //!< matrix*num returns new matrix
	Vector4F operator* (const Vector4F& rightVector) const; //!< matrix*vector returns new vector
	Vector3F operator* (const Vector3F& rightVector) const; //!< matrix*vector returns new vector
	float operator[] (loc const& cLoc); //!< operator[] overload for indexing
	float* operator[] (int index); //!< operator[] overload for indexing
	Matrix4F inverse() const; //!< calculates inverse of matrix4x4 and returns as new one
	friend Matrix4F operator* (const float& leftFloat, const Matrix4F& rightMatrix); //!< num*matrix returns new matrix
	Matrix3F convertToMatrix3() const;
	Vector3F getScale() const;
	Vector3F getPosition() const;
	Vector3F getUp() const;
	Vector3F getInvUp() const;
	Vector3F getLeft() const;
	Vector3F getInvLeft() const;
	Vector3F getBack() const;
	Vector3F getInvBack() const;
	Vector3F getForward() const;
	Vector3F getInvForward() const;
	Vector3F getAxis(int axis) const;
	Vector3F getAxisNormalized(int axis) const;

	Vector3F extractScale() const;
	QuaternionF toQuaternion() const;
	float AngleX() const;
	float AngleY() const;
	float AngleZ() const;

	void setUp(const Vector3F& axis);
	void setRight(const Vector3F& axis);
	void setForward(const Vector3F& axis);
	void setScale(const Vector3F& scale);
	void setPosition(const Vector3F& position);
	void translate(const Vector3F& position);
	void operator*=(const Matrix4F& rightMatrix);
	void setIdentity();
	void clear();
	void zeroPosition();
	void zeroScale();
	void zeroRotation();

	static Matrix4F perspective(const float &near, const float &far, const float &fov); //!< function returning perspective projection specified with given parameters
	static Matrix4F orthographic(const float &near, const float &far, const float &left, const float &right, const float &top, const float &bottom); //!< function returninng orthographic projection specified with given parameters
	static Matrix4F translate(float x, float y, float z); //!< returns translation matrix with specified translation values
	static Matrix4F scale(float x, float y, float z); //!< function returning new scale matrix with specified scale values
	static Matrix4F rotateX(float angle); //!< function returning rotation matrix with specified rotation angle along X axis
	static Matrix4F rotateY(float angle); //!< function returning rotation matrix with specified rotation angle along Y axis
	static Matrix4F rotateZ(float angle); //!< function returning rotation matrix with specified rotation angle along Z axis
	static Matrix4F rotateAngle(Vector3F& thisVector, float angle); //!< function returning rotation matrix with specified rotation angle along specified axis(vector)

	static Matrix4F nolookAt(Vector3F eye, Vector3F target, Vector3F up); //!< lookAt matrix not optimized
	static Matrix4F lookAt(Vector3F eye, Vector3F target, Vector3F up); //!< lookAt matrix optimized
	static Matrix4F FPScam(Vector3F eye, float pitch, float yaw); //!< fps camera matrix
	static Matrix4F Frustum(float l, float r, float b, float t, float n, float f); //!< frustum matrix used to produce perspective matrix with aspect ratio
	static Matrix4F OpenGLPersp(float fov, float imageAspectRatio, float near, float far); //!< perspective matrix with aspect ratio

	static Matrix4F sFrustum(float left, float right, float bottom, float top, float near, float far); //!< transposed frustum matrix used to produce transposed perspective matrix with aspect ratio
	static Matrix4F sOpenGLPersp(float fov, float imageAspectRatio, float near, float far); //!< transposed perspective matrix with aspect ratio
	static Matrix4F sPerspective(const float &near, const float &far, const float &fov); //!< function returning transposed perspective 
	static Matrix4F sTranslate(float x, float y, float z); //!< returns transposed translation matrix with specified translation values

	static Matrix4F identityMatrix(); //!< identity matrix 


private:

};