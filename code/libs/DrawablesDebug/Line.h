#pragma once
#include "MyMathLib.h"
#include "Vao.h"

class Material;

class Line
{
	
public:
	static Line* Instance();	
	void Draw(const mwm::Matrix4& Model, const mwm::Matrix4& View, const mwm::Matrix4& Projection, const unsigned int shader, float width = 4.0f);
	Material* mat;
	Vao vao;
private:
	Line();
	~Line();
	void SetUpBuffers();
	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;
	Material* localMat;
};
