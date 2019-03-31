#include "BoundingBox.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>

using namespace mwm;

BoundingBox * BoundingBox::Instance()
{
	static BoundingBox instance;

	return &instance;
}

BoundingBox::BoundingBox(){
	localMat = new Material();
	mat = localMat;
	SetUpBuffers();
}

BoundingBox::~BoundingBox()
{
	mat = nullptr;
	delete localMat;
}

void BoundingBox::SetUpBuffers()
{
	
	vao.Bind();

	Vector3 vertices[] = {
	Vector3(-0.5, -0.5, 0.5),
	Vector3(0.5, -0.5, 0.5),
	Vector3(0.5, 0.5, 0.5),
	Vector3(-0.5, 0.5, 0.5),

	Vector3(-0.5, -0.5, -0.5),
	Vector3(0.5, -0.5, -0.5),
	Vector3(0.5, 0.5, -0.5),
	Vector3(-0.5, 0.5, -0.5) };

	Vector3F verts[] = {
	Vector3F(-0.5f, -0.5f, 0.5f),
	Vector3F(0.5f, -0.5f, 0.5f),
	Vector3F(0.5f, 0.5f, 0.5f),
	Vector3F(-0.5f, 0.5f, 0.5f),

	Vector3F(-0.5f, -0.5f, -0.5f),
	Vector3F(0.5f, -0.5f, -0.5f),
	Vector3F(0.5f, 0.5f, -0.5f),
	Vector3F(-0.5f, 0.5f, -0.5f)
	};
	GLushort elements[] = {
	0, 1, 1, 2, 2, 3, 3, 0,
	4, 5, 5, 6, 6, 7, 7, 4,
	0, 4, 1, 5, 2, 6, 3, 7
	};

	vao.vertexBuffers.reserve(2);
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(Vector3F), &verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
	glEnableVertexAttribArray(0);
	vao.vertexBuffers.push_back(vertexbuffer);

	vao.indicesCount = 24;
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vao.indicesCount * sizeof(GLushort), &elements[0], GL_STATIC_DRAW);
	vao.vertexBuffers.push_back(elementbuffer);

	vao.Unbind();
}

void BoundingBox::Draw(const mwm::Matrix4& Model, const mwm::Matrix4& ViewProjection, unsigned int shader)
{
	Matrix4F MVP = (Model*ViewProjection).toFloat();
	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorValueHandle = glGetUniformLocation(shader, "MaterialColorValue");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorValueHandle, 1, &mat->color.x);

	vao.Bind();

	glDrawElements(GL_LINES, vao.indicesCount, GL_UNSIGNED_SHORT, 0);
}

