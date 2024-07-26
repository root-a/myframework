#pragma once
#include "MyMathLib.h"
#include "Vao.h"

class Material;

class Point
{
	
public:
	static Point* Instance();
	void Draw(const glm::mat4& Model, const glm::mat4& View, const glm::mat4& Projection, const unsigned int shader, float size = 10.0f);
	VertexArray vao;
	glm::vec3 color;
private:
	Point();
	~Point();
	void SetUpBuffers();
	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;
};
