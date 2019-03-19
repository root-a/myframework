#pragma once
#include "MyMathLib.h"
#include "Vao.h"

class Material;

class Plane
{
	
public:
	static Plane* Instance();
	void Draw(const mwm::Matrix4& Model, const mwm::Matrix4& View, const mwm::Matrix4& Projection, const unsigned int shader);
	Material* mat;
	Vao vao;
private:
	Plane();
	~Plane();
	void SetUpBuffers();
	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;
	Material* localMat;
};
