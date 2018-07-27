#pragma once
#include "MyMathLib.h"
#include "MinMax.h"
class Mesh;
class Material;

class Box
{
	
public:
	
	Box();
	void SetUpBuffers();
	void Draw(const mwm::Matrix4& ModelViewProjection, unsigned int wireframeShader);
	Mesh* mesh;
	Material* mat;
	
	static const unsigned short elements[36];
	static const mwm::Vector3F verts[24];

	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;
};
