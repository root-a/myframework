#pragma once
namespace mwm
{
class Vector3F;
struct loc;
class QuaternionF;

class Matrix3F
{
public:
	float _matrix[3][3];

	Matrix3F(const Matrix3F&);
	Matrix3F();
	Matrix3F(int identity);
	~Matrix3F();

	Matrix3F operator~ (); //!< transpose matrix returns new matrix
	Matrix3F operator+ (const Matrix3F& rightMatrix);
	Matrix3F& operator= (const Matrix3F& rightMatrix); //!< copy matrix returns new matrix
	bool operator== (const Matrix3F& rightMatrix); //!< check if matrices are identical
	Matrix3F operator* (const Matrix3F& rightMatrix); //!< matrix*matrix returns new matrix
	float operator() (int row, int col);//!< operator() overload for indexing
	Matrix3F operator* (const float& rightFloat); //!< matrix*num returns new matrix
	Vector3F operator* (const Vector3F& rightVector); //!< matrix*vector returns new vector
	float operator[] (loc const& cLoc); //!< operator[] overload for indexing
	float* operator[] (int index); //!< operator[] overload for indexing

	friend Matrix3F operator* (const float& leftFloat, const Matrix3F& rightMatrix); //!< num*matrix returns new matrix

	Matrix3F inverse() const; //!< calculates inverse of matrix4x4 and returns as new one
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

	void setUp(const Vector3F& axis);
	void setRight(const Vector3F& axis);
	void setForward(const Vector3F& axis);
	void setAxes(const Vector3F& right, const Vector3F& up, const Vector3F& forward);

	void setSkewSymmetric(const Vector3F& vector);

	static Matrix3F CuboidInertiaTensor(float mass, Vector3F dimensions);
	static float det(float a, float b, float c, float d, float e, float f, float g, float h, float i); //!< calculates determinant of 3x3 matrix
	static Matrix3F translate(float x, float y, float z); //!< returns translation matrix with specified translation values
	static Matrix3F scale(float x, float y, float z); //!< function returning new scale matrix with specified scale values
	static Matrix3F rotateX(float angle); //!< function returning rotation matrix with specified rotation angle along X axis
	static Matrix3F rotateY(float angle); //!< function returning rotation matrix with specified rotation angle along Y axis
	static Matrix3F rotateZ(float angle); //!< function returning rotation matrix with specified rotation angle along Z axis
	static Matrix3F rotateAngle(Vector3F& thisVector, float angle); //!< function returning rotation matrix with specified rotation angle along specified axis(vector)
	//static Matrix3F identityMatrix(); //!< identity matrix 


private:

};
}
