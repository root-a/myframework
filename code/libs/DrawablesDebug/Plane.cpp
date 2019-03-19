#include "Plane.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>

using namespace mwm;

Plane * Plane::Instance()
{
	static Plane instance;

	return &instance;
}

Plane::Plane()
{
	localMat = new Material();
	mat = localMat;
	SetUpBuffers();
}

Plane::~Plane()
{
	mat = nullptr;
	delete localMat;
}


void Plane::SetUpBuffers()
{
	//Create VAO
	glGenVertexArrays(1, &vao.vaoHandle);
	//Bind VAO
	vao.Bind();

	vao.vertexBuffers.reserve(2);

	unsigned short elements[] = { 0, 1, 2, 2, 1, 3 };
	Vector3F vertices[] = {Vector3F(-1.f, -1.f, 0.f),Vector3F(1.f, -1.f, 0.f),Vector3F(-1.f, 1.f, 0.f),Vector3F(1.f, 1.f, 0.f)};

	GLuint vertexbuffer;
	// 1rst attribute buffer : vertices
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vector3F), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
	glEnableVertexAttribArray(0);
	vao.vertexBuffers.push_back(vertexbuffer);

	vao.indicesCount = 6;
	GLuint elementbuffer;
	// 4th element buffer Generate a buffer for the indices as well
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vao.indicesCount * sizeof(GLushort), &elements[0], GL_STATIC_DRAW);
	vao.vertexBuffers.push_back(elementbuffer);

	//Unbind the VAO now that the VBOs have been set up
	vao.Unbind();
}

void Plane::Draw(const Matrix4& Model, const Matrix4& View, const Matrix4& Projection, const GLuint shader)
{
	Matrix4F MVP = (Model*View*Projection).toFloat();

	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorValueHandle = glGetUniformLocation(shader, "MaterialColorValue");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorValueHandle, 1, &mat->color.x);

	//bind vao before drawing
	vao.Unbind();

	// Draw the triangles !
	glDrawElements(GL_TRIANGLES, vao.indicesCount, GL_UNSIGNED_SHORT, (void*)0); // mode, count, type, element array buffer offset
}