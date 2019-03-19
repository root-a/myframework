#include "Box.h"
#include "Material.h"
#include "Texture.h"
#include <algorithm>
#include <GL/glew.h>

using namespace mwm;

Box * Box::Instance()
{
	static Box instance;

	return &instance;
}

Box::Box(){
	localMat = new Material();
	mat = localMat;
	SetUpBuffers();
}

Box::~Box()
{
	mat = nullptr;
	delete localMat;
}

void Box::SetUpBuffers()
{
	glGenVertexArrays(1, &vao.vaoHandle);
	vao.Bind();

	vao.vertexBuffers.reserve(2);

	Vector3F verts[] = {
	Vector3F(-1.f, -1.f, 1.f),
	Vector3F(-1.f, 1.f, 1.f),
	Vector3F(1.f, -1.f, 1.f),
	Vector3F(1.f, 1.f, 1.f),

	Vector3F(-1.f, 1.f, 1.f),
	Vector3F(-1.f, 1.f, -1.f),
	Vector3F(1.f, 1.f, 1.f),
	Vector3F(1.f, 1.f, -1.f),

	Vector3F(-1.f, 1.f, -1.f),
	Vector3F(-1.f, -1.f, -1.f),
	Vector3F(1.f, 1.f, -1.f),
	Vector3F(1.f, -1.f, -1.f),

	Vector3F(-1.f, -1.f, -1.f),
	Vector3F(-1.f, -1.f, 1.f),
	Vector3F(1.f, -1.f, -1.f),
	Vector3F(1.f, -1.f, 1.f),

	Vector3F(1.f, -1.f, 1.f),
	Vector3F(1.f, 1.f, 1.f),
	Vector3F(1.f, -1.f, -1.f),
	Vector3F(1.f, 1.f, -1.f),

	Vector3F(-1.f, -1.f, -1.f),
	Vector3F(-1.f, 1.f, -1.f),
	Vector3F(-1.f, -1.f, 1.f),
	Vector3F(-1.f, 1.f, 1.f)
	};

	GLushort elements[] = {
		0, 1, 2,
		1, 3, 2,
		4, 5, 6,
		5, 7, 6,
		8, 9, 10,
		9, 11, 10,
		12, 13, 14,
		13, 15, 14,
		16, 17, 18,
		17, 19, 18,
		20, 21, 22,
		21, 23, 22
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(Vector3F), &verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
	glEnableVertexAttribArray(0);
	vao.vertexBuffers.push_back(vertexbuffer);

	vao.indicesCount = 36;
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vao.indicesCount * sizeof(GLushort), &elements[0], GL_STATIC_DRAW);
	vao.vertexBuffers.push_back(elementbuffer);

	vao.Unbind();
}

void Box::Draw(const mwm::Matrix4& ModelViewProjection, unsigned int shader)
{
	Matrix4F MVP = ModelViewProjection.toFloat();
	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorHandle = glGetUniformLocation(shader, "MaterialColor");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorHandle, 1, &mat->color.x);

	mat->ActivateAndBind();

	vao.Bind();
	glDrawElements(GL_TRIANGLES, vao.indicesCount, GL_UNSIGNED_SHORT, 0);
}