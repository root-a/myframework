#include "BoundingBox.h"
#include "Mesh.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>

using namespace mwm;

BoundingBox::BoundingBox(){

	mat = new Material();
	SetUpBuffers();
}

const Vector3 BoundingBox::vertices[8] = {
	Vector3(-0.5, -0.5, 0.5),
	Vector3(0.5, -0.5, 0.5),
	Vector3(0.5, 0.5, 0.5),
	Vector3(-0.5, 0.5, 0.5),

	Vector3(-0.5, -0.5, -0.5),
	Vector3(0.5, -0.5, -0.5),
	Vector3(0.5, 0.5, -0.5),
	Vector3(-0.5, 0.5, -0.5)
};

const Vector3F BoundingBox::verts[8] = {
	Vector3F(-0.5f, -0.5f, 0.5f),
	Vector3F(0.5f, -0.5f, 0.5f),
	Vector3F(0.5f, 0.5f, 0.5f),
	Vector3F(-0.5f, 0.5f, 0.5f),

	Vector3F(-0.5f, -0.5f, -0.5f),
	Vector3F(0.5f, -0.5f, -0.5f),
	Vector3F(0.5f, 0.5f, -0.5f),
	Vector3F(-0.5f, 0.5f, -0.5f)
};

const GLushort BoundingBox::elements[] = {
	0, 1, 1, 2, 2, 3, 3, 0,
	4, 5, 5, 6, 6, 7, 7, 4,
	0, 4, 1, 5, 2, 6, 3, 7
};



void BoundingBox::SetUpBuffers()
{
	mesh = new Mesh();

	glGenVertexArrays(1, &mesh->vaoHandle);
	glBindVertexArray(mesh->vaoHandle);

	glGenBuffers(1, &mesh->vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(Vector3F), &verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &mesh->elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(GLushort), &elements[0], GL_STATIC_DRAW);
	mesh->indicesSize = 24;

	glBindVertexArray(0);

}

void BoundingBox::Draw(const mwm::Matrix4& Model, const mwm::Matrix4& ViewProjection, unsigned int shader)
{
	Matrix4F MVP = (Model*ViewProjection).toFloat();
	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorValueHandle = glGetUniformLocation(shader, "MaterialColorValue");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorValueHandle, 1, &this->mat->color.x);

	glBindVertexArray(this->mesh->vaoHandle);

	glDrawElements(GL_LINES, mesh->indicesSize, GL_UNSIGNED_SHORT, 0);
}

MinMax BoundingBox::CalcValuesInWorld(const Matrix3& modelM, const Vector3& position)
{
	Vector3 maxValuesW = modelM * vertices[0];
	Vector3 minValuesW = modelM * vertices[0];
	Vector3 currentVertex;
	for (int i = 0; i < 8; ++i)
	{
		currentVertex = modelM * vertices[i];
		maxValuesW[0] = std::max(maxValuesW[0], currentVertex[0]);
		minValuesW[0] = std::min(minValuesW[0], currentVertex[0]);
		maxValuesW[1] = std::max(maxValuesW[1], currentVertex[1]);
		minValuesW[1] = std::min(minValuesW[1], currentVertex[1]);
		maxValuesW[2] = std::max(maxValuesW[2], currentVertex[2]);
		minValuesW[2] = std::min(minValuesW[2], currentVertex[2]);
	}
	MinMax mm;
	mm.max = maxValuesW + position;
	mm.min = minValuesW + position;
	return mm;
}

mwm::MinMax BoundingBox::CalcValuesInWorld(const mwm::Matrix4 & modelMatrix)
{
	Vector4 maxValuesW = modelMatrix * vertices[0];
	Vector4 minValuesW = modelMatrix * vertices[0];
	Vector4 currentVertex;
	for (int i = 0; i < 8; ++i)
	{
		currentVertex = modelMatrix * vertices[i];
		maxValuesW.x = std::max(maxValuesW.x, currentVertex.x);
		minValuesW.x = std::min(minValuesW.x, currentVertex.x);
		maxValuesW.y = std::max(maxValuesW.y, currentVertex.y);
		minValuesW.y = std::min(minValuesW.y, currentVertex.y);
		maxValuesW.z = std::max(maxValuesW.z, currentVertex.z);
		minValuesW.z = std::min(minValuesW.z, currentVertex.z);
	}
	MinMax mm;
	mm.max = Vector3(maxValuesW.x, maxValuesW.y, maxValuesW.z);
	mm.min = Vector3(minValuesW.x, minValuesW.y, minValuesW.z);
	return mm;
}
