#pragma once
#include "MyMathLib.h"
#include "Vao.h"

class Material;

class Line
{
	
public:
	static Line* Instance();	
	void Draw(const glm::mat4& Model, const glm::mat4& View, const glm::mat4& Projection, const unsigned int shader);
	VertexArray vao;
	glm::vec3 color;
private:
	Line();
	~Line();
	void SetUpBuffers();
	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;
};
