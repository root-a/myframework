#pragma once
#include "MyMathLib.h"
#include "Vao.h"

class Material;

class Point
{
	
public:
	static Point* Instance();
	void Draw(const Matrix4& Model, const Matrix4& View, const Matrix4& Projection, const unsigned int shader, float size = 10.0f);
	Vao vao;
	Vector3F color;
private:
	Point();
	~Point();
	void SetUpBuffers();
	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;
};
