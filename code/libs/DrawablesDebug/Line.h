#pragma once
#include "MyMathLib.h"
#include "Vao.h"

class Material;

class Line
{
	
public:
	static Line* Instance();	
	void Draw(const Matrix4& Model, const Matrix4& View, const Matrix4& Projection, const unsigned int shader);
	Vao vao;
	Vector3F color;
private:
	Line();
	~Line();
	void SetUpBuffers();
	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;
};
