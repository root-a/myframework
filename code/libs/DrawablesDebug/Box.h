#pragma once
#include "MyMathLib.h"
#include "MinMax.h"
#include "Vao.h"
class Material;
class Texture;

class Box
{
	
public:
	static Box* Instance();
	void Draw(const Matrix4& ModelViewProjection, unsigned int shader);
	Vao vao;
	Texture* tex;
	Vector3F color;
private:
	Box();
	~Box();
	void SetUpBuffers();
	unsigned int MatrixHandle;
	unsigned int MaterialColorHandle;
};
