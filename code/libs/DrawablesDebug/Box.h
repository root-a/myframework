#pragma once
#include "MyMathLib.h"
#include "MinMax.h"
#include "Vao.h"
class Material;

class Box
{
	
public:
	static Box* Instance();
	void Draw(const mwm::Matrix4& ModelViewProjection, unsigned int shader);
	Material* mat;
	Vao vao;
private:
	Box();
	~Box();
	void SetUpBuffers();
	unsigned int MatrixHandle;
	unsigned int MaterialColorHandle;
	Material* localMat;
};
