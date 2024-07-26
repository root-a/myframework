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
	void Draw(const glm::mat4& ModelViewProjection, unsigned int shader);
	VertexArray vao;
	Texture* tex;
	glm::vec3 color;
private:
	Box();
	~Box();
	void SetUpBuffers();
	unsigned int MatrixHandle;
	unsigned int MaterialColorHandle;
};
