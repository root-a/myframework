#pragma once
#include "MyMathLib.h"
#include "MinMax.h"
#include "Vao.h"
#include "Material.h"

class BoundingBox
{
	
public:
	static BoundingBox* Instance();
	void Draw(const mwm::Matrix4& Model, const mwm::Matrix4& ViewProjection, unsigned int wireframeShader);
	Material* mat;
	Vao vao;
private:
	BoundingBox();
	~BoundingBox();
	void SetUpBuffers();
	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;
	Material* localMat;
};