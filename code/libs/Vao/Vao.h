#pragma once
#include "MyMathLib.h"
#include <string>
#include <vector>

class OBJ;

class Vao
{
public:
	Vao();
	~Vao();
	void Bind();
	void Unbind();
	unsigned int vaoHandle = -1;
	std::vector<unsigned int> vertexBuffers;
	unsigned int indicesCount;
private:

};

