#pragma once
#include "MyMathLib.h"
#include "Vao.h"

class Material;

class Plane
{
	
public:
	static Plane* Instance();
	void Draw(const glm::mat4& Model, const glm::mat4& View, const glm::mat4& Projection, const unsigned int shader);
	VertexArray vao;
	glm::vec3 color;
private:
	Plane();
	~Plane();
	void SetUpBuffers();
	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;
};
