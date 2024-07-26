#include <cmath>
#include <memory.h>
#include "Matrix3F.h"
#include "Matrix4F.h"
#include "Vector3F.h"
#include "mLoc.h"
#include "QuaternionF.h"
#include "Matrix3.h"
#include "Matrix4.h"
#include <cstring>

/*! \fn in constructor matrix values are set to 0 with memset*/
Matrix3F::Matrix3F()
{
	memset(_matrix, 0, sizeof _matrix);
}

Matrix3F::Matrix3F(int identity)
{
	memset(_matrix, 0, sizeof _matrix);
	_matrix[0][0] = 1.f;
	_matrix[1][1] = 1.f;
	_matrix[2][2] = 1.f;
}

Matrix3F::~Matrix3F()
{

}

/*! \fn identity matrix */
/*
Matrix3F Matrix3F::identityMatrix()
{
	Matrix3F temp;
	temp._matrix[0][0] = 1.f;
	temp._matrix[1][1] = 1.f;
	temp._matrix[2][2] = 1.f;
	return temp;
}
*/

/*! \fn operator[] overload for indexing */
float* Matrix3F::operator[] (int index)
{
	return _matrix[index];
}

/*! \fn operator[] overload for indexing */
float Matrix3F::operator[](loc const& mLoc)
{
	return _matrix[mLoc.x][mLoc.y];
}

/*! \fn operator() overload for indexing*/
float Matrix3F::operator() (int row, int col)
{
	return _matrix[row][col];
}

/*! \fn transpose matrix returns new matrix*/
Matrix3F Matrix3F::operator~ ()
{
	Matrix3F realTemp;
	for (int r = 0; r < 3; r++)
	{
		for (int c = 0; c < 3; c++)
		{
			//cout << first._matrix[r][c] << " result är:" <<  result._matrix[r][c] << endl;
			realTemp._matrix[r][c] = _matrix[c][r];
		}
	}
	return realTemp;
}

Matrix3F Matrix3F::operator+(const Matrix3F& right)
{
	Matrix3F realTemp;
	for (int r = 0; r < 3; r++)
	{
		for (int c = 0; c < 3; c++)
		{
			realTemp._matrix[r][c] = _matrix[r][c] + right._matrix[r][c];
		}
	}
	return realTemp;
}

/*! \fn num*matrix returns new matrix*/
Matrix3F operator*(const float& left, const Matrix3F& right)
{
	Matrix3F temp;
	for (int i = 0; i < 3; i++)
	{
		for (int c = 0; c < 3; c++)
		{
			temp._matrix[i][c] = left * right._matrix[i][c];
		}
	}
	return temp;
}

/*! \fn matrix*num returns new matrix*/
Matrix3F Matrix3F::operator* (const float& right)
{
	Matrix3F temp;
	for (int i = 0; i < 3; i++)
	{
		for (int c = 0; c < 3; c++)
		{
			temp._matrix[i][c] = _matrix[i][c] * right;

		}
	}
	return temp;

}

/*! \fn matrix*matrix returns new matrix*/
Matrix3F Matrix3F::operator* (const Matrix3F& right) // matrix multi
{
	Matrix3F temp;

	//multiply
	for (int r = 0; r < 3; r++)
	{
		for (int c = 0; c < 3; c++)
		{
			for (int k = 0; k < 3; k++)
			{
				temp._matrix[r][c] = temp._matrix[r][c] + _matrix[r][k] * right._matrix[k][c];

			}
		}
	}
	return temp;
}

/*! \fn matrix*vector returns new vector*/
Vector3F Matrix3F::operator* (const Vector3F& right) // matrix multi
{
	float vx = right.x;
	float vy = right.y;
	float vz = right.z;

	float _x = _matrix[0][0] * vx + _matrix[1][0] * vy + _matrix[2][0] * vz;
	float _y = _matrix[0][1] * vx + _matrix[1][1] * vy + _matrix[2][1] * vz;
	float _z = _matrix[0][2] * vx + _matrix[1][2] * vy + _matrix[2][2] * vz;
	return Vector3F(_x, _y, _z);
}

Matrix3F & Matrix3F::operator=(const Matrix3 & right)
{
	for (int r = 0; r < 3; r++)
	{
		for (int c = 0; c < 3; c++)
		{
			_matrix[r][c] = (float)right._matrix[r][c];

		}
	}
	return *this;
}

/*! \fn assign matrix returns this matrix*/
Matrix3F& Matrix3F::operator= (const Matrix4& right)
{
	for (int r = 0; r < 3; r++)
	{
		for (int c = 0; c < 3; c++)
		{
			_matrix[r][c] = (float)right._matrix[r][c];

		}
	}
	return *this;
}

Matrix3F& Matrix3F::operator= (const Matrix4F& right)
{
	for (int r = 0; r < 3; r++)
	{
		for (int c = 0; c < 3; c++)
		{
			_matrix[r][c] = right._matrix[r][c];

		}
	}
	return *this;
}

/*! \fn check if matrices are identical*/
bool Matrix3F::operator== (const Matrix3F& right)
{
	return !std::memcmp((void*)this, (void*)&right, sizeof(Matrix3F));
	/*
	for (int r = 0; r < 3; r++)
	{
		for (int c = 0; c < 3; c++)
		{
			if (_matrix[r][c] != right._matrix[r][c])
			{
				return false;
			}
		}
	}
	return true;
	*/
}

/*! \fn function returning rotation matrix with specified rotation angle along X axis*/
Matrix3F Matrix3F::rotateX(float angle)
{
	float sAngle = -angle;
	float PI = 3.14159265f;
	float cosAng = cosf(sAngle * PI / 180.0f);
	float sinAng = sinf(sAngle * PI / 180.0f);
	Matrix3F rotationMatrixX;
	//row1
	rotationMatrixX._matrix[0][0] = 1.f;
	//rotationMatrixX._matrix[0][1] = 0;
	//rotationMatrixX._matrix[0][2] = 0;
	//rotationMatrixX._matrix[0][3] = 0;
	//row2
	//rotationMatrixX._matrix[1][0] = 0;
	rotationMatrixX._matrix[1][1] = cosAng;
	rotationMatrixX._matrix[1][2] = -sinAng;
	//rotationMatrixX._matrix[1][3] = 0;
	//row3
	//rotationMatrixX._matrix[2][0] = 0;
	rotationMatrixX._matrix[2][1] = sinAng;
	rotationMatrixX._matrix[2][2] = cosAng;
	//rotationMatrixX._matrix[2][3] = 0;
	//row4
	//rotationMatrixX._matrix[3][0] = 0;
	//rotationMatrixX._matrix[3][1] = 0;
	//rotationMatrixX._matrix[3][2] = 0;
	//rotationMatrixX._matrix[3][3] = 1;
	return rotationMatrixX;

}

/*! \fn function returning rotation matrix with specified rotation angle along Y axis*/
Matrix3F Matrix3F::rotateY(float angle)
{
	float sAngle = -angle;
	float PI = 3.14159265f;
	float cosAng = cosf(sAngle * PI / 180.0f);
	float sinAng = sinf(sAngle * PI / 180.0f);
	Matrix3F rotationMatrixY;
	//row1
	rotationMatrixY._matrix[0][0] = cosAng;
	//rotationMatrixY._matrix[0][1] = 0;
	rotationMatrixY._matrix[0][2] = sinAng;
	//row2
	//rotationMatrixY._matrix[1][0] = 0;
	rotationMatrixY._matrix[1][1] = 1.f;
	//rotationMatrixY._matrix[1][2] = 0;
	//row3
	rotationMatrixY._matrix[2][0] = -sinAng;
	//rotationMatrixY._matrix[2][1] = 0;
	rotationMatrixY._matrix[2][2] = cosAng;
	return rotationMatrixY;
}

/*! \fn function returning rotation matrix with specified rotation angle along Z axis*/
Matrix3F Matrix3F::rotateZ(float angle)
{
	float sAngle = -angle;
	float PI = 3.14159265f;
	float cosAng = cosf(sAngle * PI / 180.0f);
	float sinAng = sinf(sAngle * PI / 180.0f);
	Matrix3F rotationMatrixZ;
	//row1
	rotationMatrixZ._matrix[0][0] = cosAng;
	rotationMatrixZ._matrix[0][1] = -sinAng;
	//rotationMatrixZ._matrix[0][2] = 0;
	//row2
	rotationMatrixZ._matrix[1][0] = sinAng;
	rotationMatrixZ._matrix[1][1] = cosAng;
	//rotationMatrixZ._matrix[1][2] = 0;
	//row3
	//rotationMatrixZ._matrix[2][0] = 0;
	//rotationMatrixZ._matrix[2][1] = 0;
	rotationMatrixZ._matrix[2][2] = 1.f;

	return rotationMatrixZ;
}

/*! \fn function returning rotation matrix with specified rotation angle along specified axis(vector)*/
Matrix3F Matrix3F::rotateAngle(Vector3F& thisVector, float angle)
{
	float sAngle = -angle;
	float PI = 3.14159265f;
	float cosAng = cosf(sAngle * PI / 180.0f);
	float sinAng = sinf(sAngle * PI / 180.0f);
	float T = 1 - cosAng;
	Vector3F normalizedVector = thisVector.normalize();
	float x = normalizedVector.x;
	float y = normalizedVector.y;
	float z = normalizedVector.z;
	Matrix3F rotationMatrix;
	//row1
	rotationMatrix._matrix[0][0] = cosAng + (x*x) * T;
	rotationMatrix._matrix[0][1] = x*y * T - z * sinAng;
	rotationMatrix._matrix[0][2] = x*z * T + y * sinAng;
	//row2
	rotationMatrix._matrix[1][0] = y*x * T + z * sinAng;
	rotationMatrix._matrix[1][1] = cosAng + y*y * T;
	rotationMatrix._matrix[1][2] = y*z * T - x * sinAng;
	//row3
	rotationMatrix._matrix[2][0] = z*x * T - y * sinAng;
	rotationMatrix._matrix[2][1] = z*y * T + x * sinAng;
	rotationMatrix._matrix[2][2] = cosAng + z*z * T;
	return rotationMatrix;
}

/*! \fn returns translation matrix with specified translation values*/
Matrix3F Matrix3F::translate(float x, float y, float z)
{
	Matrix3F translation;
	translation._matrix[0][0] = 1.f;
	translation._matrix[1][1] = 1.f;
	translation._matrix[2][2] = 1.f;
	//translation._matrix[3][3] = 1;
	translation._matrix[0][2] = x;
	translation._matrix[1][2] = y;
	translation._matrix[2][2] = z;
	return translation;
}

/*! \fn function returning new scale matrix with specified scale values*/
Matrix3F Matrix3F::scale(float x, float y, float z)
{
	Matrix3F scaling;
	scaling._matrix[0][0] = x;
	scaling._matrix[1][1] = y;
	scaling._matrix[2][2] = z;
	//scaling._matrix[3][3] = 1;
	return scaling;
}

/*! \fn calculates inverse of matrix3x3 and returns as new one*/
Matrix3F Matrix3F::inverse() const
{
	//find determinant
	float a, b, c, d, e, f, g, h, i;
	a = _matrix[0][0]; 	b = _matrix[0][1]; 	c = _matrix[0][2];
	d = _matrix[1][0]; 	e = _matrix[1][1]; 	f = _matrix[1][2];
	g = _matrix[2][0]; 	h = _matrix[2][1]; 	i = _matrix[2][2];

	//double det = a*e*i + b*f*g + c*d*h - c*e*g - b*d*i - a*f*h; //or a(ei-fh)-b(id-fg)+c(dh-eg)
	float det = a * (e*i - f * h) - b * (i*d - f * g) + c * (d*h - e * g);
	//cout << det << endl;
	Matrix3F temp;
	//matrix of minor, transpose and change signs in one
	temp._matrix[0][0] = (e*i - f*h);	temp._matrix[0][1] = -(b*i - c*h);	temp._matrix[0][2] = (b*f - c*e);			// + - +
	temp._matrix[1][0] = -(d*i - f*g);	temp._matrix[1][1] = (a*i - c*g);	temp._matrix[1][2] = -(a*f - c*d);			// - + -
	temp._matrix[2][0] = (d*h - e*g);	temp._matrix[2][1] = -(a*h - b*g);	temp._matrix[2][2] = (a*e - b*d);			// + - +

	float oneByDet = 1 / det;

	temp = oneByDet*temp;

	return temp;
}

/*! \fn calculates determinant of 3x3 matrix*/
float Matrix3F::det(float a, float b, float c, float d, float e, float f, float g, float h, float i)
{
	return a*(e*i - f*h) - b*(i*d - f*g) + c*(d*h - e*g);
	//simplified: return (a*e*i + b*f*g + c*d*h - c*e*g - b*d*i - a*f*h);
}

Matrix3F Matrix3F::CuboidInertiaTensor(Vector3F dimensions)
{
	Matrix3F I;
	I[0][0] = (1.f / 12.f) * (dimensions.vect[1] * dimensions.vect[1] + dimensions.vect[2] * dimensions.vect[2]);
	I[1][1] = (1.f / 12.f) * (dimensions.vect[0] * dimensions.vect[0] + dimensions.vect[2] * dimensions.vect[2]);
	I[2][2] = (1.f / 12.f) * (dimensions.vect[0] * dimensions.vect[0] + dimensions.vect[1] * dimensions.vect[1]);

	return I;
}

Vector3F Matrix3F::getLeft() const
{
	return Vector3F(_matrix[0][0], _matrix[0][1], _matrix[0][2]).normalize();
}

Vector3F Matrix3F::getInvLeft() const
{
	return Vector3F(_matrix[0][0], _matrix[1][0], _matrix[2][0]).normalize();
}

Vector3F Matrix3F::getUp() const
{
	return Vector3F(_matrix[1][0], _matrix[1][1], _matrix[1][2]).normalize();
}

Vector3F Matrix3F::getInvUp() const
{
	return Vector3F(_matrix[0][1], _matrix[1][1], _matrix[2][1]).normalize();
}

Vector3F Matrix3F::getBack() const
{
	return Vector3F(_matrix[2][0] * -1.f, _matrix[2][1] * -1.f, _matrix[2][2] * -1.f).normalize();
}

Vector3F Matrix3F::getInvBack() const
{
	return Vector3F(_matrix[0][2] * -1.f, _matrix[1][2] * -1.f, _matrix[2][2] * -1.f).normalize();
}

Vector3F Matrix3F::getForward() const
{
	return Vector3F(_matrix[2][0], _matrix[2][1], _matrix[2][2]).normalize();
}

Vector3F Matrix3F::getInvForward() const
{
	return Vector3F(_matrix[0][2], _matrix[1][2], _matrix[2][2]).normalize();
}

void Matrix3F::setUp(const Vector3F& axis)
{
	_matrix[1][0] = axis.x;
	_matrix[1][1] = axis.y;
	_matrix[1][2] = axis.z;
}

void Matrix3F::setRight(const Vector3F& axis)
{
	_matrix[0][0] = axis.x;
	_matrix[0][1] = axis.y;
	_matrix[0][2] = axis.z;
}

void Matrix3F::setForward(const Vector3F& axis)
{
	_matrix[2][0] = axis.x;
	_matrix[2][1] = axis.y;
	_matrix[2][2] = axis.z;
}

void Matrix3F::setAxes(const Vector3F& right, const Vector3F& up, const Vector3F& forward)
{
	_matrix[0][0] = right.x;
	_matrix[0][1] = right.y;
	_matrix[0][2] = right.z;

	_matrix[1][0] = up.x;
	_matrix[1][1] = up.y;
	_matrix[1][2] = up.z;

	_matrix[2][0] = forward.x;
	_matrix[2][1] = forward.y;
	_matrix[2][2] = forward.z;
}

void Matrix3F::clear()
{
	memset(_matrix, 0, sizeof _matrix);
}

void Matrix3F::setSkewSymmetric(const Vector3F& vector)
{
	_matrix[0][0] = _matrix[1][1] = _matrix[2][2] = 0;
	_matrix[0][1] = -vector.z;
	_matrix[0][2] = vector.y;
	_matrix[1][0] = vector.z;
	_matrix[1][2] = -vector.x;
	_matrix[2][0] = -vector.y;
	_matrix[2][1] = vector.x;
}

Vector3F Matrix3F::getAxis(int axis) const
{
	return Vector3F(_matrix[axis][0], _matrix[axis][1], _matrix[axis][2]);
}

Vector3F Matrix3F::getAxisNormalized(int axis) const
{
	return Vector3F(_matrix[axis][0], _matrix[axis][1], _matrix[axis][2]).normalize();
}

Vector3F Matrix3F::extractScale() const
{
	float scaleX = Vector3F(_matrix[0][0], _matrix[0][1], _matrix[0][2]).lengt();
	float scaleY = Vector3F(_matrix[1][0], _matrix[1][1], _matrix[1][2]).lengt();
	float scaleZ = Vector3F(_matrix[2][0], _matrix[2][1], _matrix[2][2]).lengt();
	return Vector3F(scaleX, scaleY, scaleZ);
}

QuaternionF Matrix3F::toQuaternion() const
{
	float tr = _matrix[0][0] + _matrix[1][1] + _matrix[2][2];
	QuaternionF temp;
	if (tr > 0.f) {
		float S = sqrtf(tr + 1.f) * 2.f; // S=4*qw
		temp.w = 0.25f * S;
		temp.x = (_matrix[1][2] - _matrix[2][1]) / S;
		temp.y = (_matrix[2][0] - _matrix[0][2]) / S;
		temp.z = (_matrix[0][1] - _matrix[1][0]) / S;
	}

	else if ((_matrix[0][0] > _matrix[1][1])&(_matrix[0][0] > _matrix[2][2])) {
		float S = sqrtf(1.f + _matrix[0][0] - _matrix[1][1] - _matrix[2][2]) * 2.f; // S=4*qx
		temp.w = (_matrix[1][2] - _matrix[2][1]) / S;
		temp.x = 0.25f * S;
		temp.y = (_matrix[1][0] + _matrix[0][1]) / S;
		temp.z = (_matrix[2][0] + _matrix[0][2]) / S;
	}

	else if (_matrix[1][1] > _matrix[2][2]) {
		float S = sqrtf(1.f + _matrix[1][1] - _matrix[0][0] - _matrix[2][2]) * 2.f; // S=4*qy
		temp.w = (_matrix[2][0] - _matrix[0][2]) / S;
		temp.x = (_matrix[1][0] + _matrix[0][1]) / S;
		temp.y = 0.25f * S;
		temp.z = (_matrix[2][1] + _matrix[1][2]) / S;
	}

	else {
		float S = sqrtf(1.f + _matrix[2][2] - _matrix[0][0] - _matrix[1][1]) * 2.f; // S=4*qz
		temp.w = (_matrix[0][1] - _matrix[1][0]) / S;
		temp.x = (_matrix[2][0] + _matrix[0][2]) / S;
		temp.y = (_matrix[2][1] + _matrix[1][2]) / S;
		temp.z = 0.25f * S;
	}
	return temp;
}

float Matrix3F::AngleX() const
{
	float PI = 3.14159265f;
	float decompRotX = (atan2(_matrix[2][1], _matrix[2][2]) * 180.0) / PI;
	if (decompRotX > 180.0f) {
		decompRotX -= 360.0f;
	}
	else if (decompRotX < -180.0f) {
		decompRotX += 360.0f;
	}
	decompRotX = -1.0f * decompRotX;
	return decompRotX;
}

float Matrix3F::AngleY() const
{
	float PI = 3.14159265f;
	float decompRotY = (atan2(-_matrix[2][0], sqrt((_matrix[2][1] * _matrix[2][1]) + (_matrix[2][2] * _matrix[2][2]))) * 180.0) / PI;
	if (decompRotY > 180.0f) {
		decompRotY -= 360.0f;
	}
	else if (decompRotY < -180.0f) {
		decompRotY += 360.0f;
	}
	decompRotY = -1.0f * decompRotY;
	return decompRotY;
}

float Matrix3F::AngleZ() const
{
	float PI = 3.14159265f;
	float decompRotZ = (atan2(_matrix[1][0], _matrix[0][0]) * 180.0) / PI;
	if (decompRotZ > 180.0f) {
		decompRotZ -= 360.0f;
	}
	else if (decompRotZ < -180.0f) {
		decompRotZ += 360.0f;
	}
	decompRotZ = -1.0f * decompRotZ;
	return decompRotZ;
}

/*! \fn converts float matrix to double */
Matrix3 Matrix3F::toDouble() const
{
	Matrix3 temp;
	temp._matrix[0][0] = _matrix[0][0]; 	temp._matrix[0][1] = _matrix[0][1]; 	temp._matrix[0][2] = _matrix[0][2]; 	temp._matrix[0][3] = _matrix[0][3];
	temp._matrix[1][0] = _matrix[1][0]; 	temp._matrix[1][1] = _matrix[1][1]; 	temp._matrix[1][2] = _matrix[1][2]; 	temp._matrix[1][3] = _matrix[1][3];
	temp._matrix[2][0] = _matrix[2][0]; 	temp._matrix[2][1] = _matrix[2][1]; 	temp._matrix[2][2] = _matrix[2][2]; 	temp._matrix[2][3] = _matrix[2][3];
	return temp;
}