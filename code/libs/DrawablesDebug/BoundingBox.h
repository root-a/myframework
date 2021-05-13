#pragma once
#include "MyMathLib.h"
#include "MinMax.h"
#include "Vao.h"

class BoundingBox
{
	
public:
	static BoundingBox* Instance();
	void Draw(const Matrix4& Model, const Matrix4& ViewProjection, unsigned int wireframeShader);
	Vao vao;
	Vector3F color;
private:
	BoundingBox();
	~BoundingBox();
	void SetUpBuffers();
	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;
};