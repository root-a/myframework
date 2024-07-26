#pragma once
#include "MyMathLib.h"
#include "MinMax.h"
#include "Vao.h"
#include "Material.h"

class BoundingBox
{
	
public:
	static BoundingBox* Instance();
	void Draw(const glm::mat4& Model, const glm::mat4& ProjectionView, unsigned int wireframeShader);
	VertexArray vao;
	glm::vec3 color;
private:
	BoundingBox();
	~BoundingBox();
	void SetUpBuffers();
	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;
};