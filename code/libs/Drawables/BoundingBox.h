#pragma once
#include "MyMathLib.h"
#include "MinMax.h"
class Mesh;
class Material;

class BoundingBox
{
	
public:
	
	BoundingBox();
	void SetUpBuffers();
	void Draw(const mwm::Matrix4& Model, const mwm::Matrix4& ViewProjection, unsigned int wireframeShader);
	static mwm::MinMax CalcValuesInWorld(const mwm::Matrix3& modelMatrix, const mwm::Vector3& position);
	static mwm::MinMax CalcValuesInWorld(const mwm::Matrix4& modelMatrix);
	Mesh* mesh;
	Material* mat;
	
	static const unsigned short elements[24];
	static const mwm::Vector3 vertices[8];
	static const mwm::Vector3F verts[8];

	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;
};
