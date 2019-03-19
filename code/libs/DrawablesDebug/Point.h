#pragma once
#include "MyMathLib.h"
#include "Vao.h"

class Material;

class Point
{
	
public:
	static Point* Instance();
	void Draw(const mwm::Matrix4& Model, const mwm::Matrix4& View, const mwm::Matrix4& Projection, const unsigned int shader, float size = 10.0f);
	Material* mat;
	Vao vao;
private:
	Point();
	~Point();
	void SetUpBuffers();
	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;
	Material* localMat;
};
