#include "Plane.h"
#include <algorithm>
#include <GL/glew.h>

Plane * Plane::Instance()
{
	static Plane instance;

	return &instance;
}

Plane::Plane()
{
	color.x = 1;
	color.y = 1;
	color.z = 0;
	SetUpBuffers();
}

Plane::~Plane()
{
}

void Plane::SetUpBuffers()
{
	vao.vertexBuffers.reserve(2);

	unsigned short elements[] = { 0, 1, 2, 2, 1, 3 };
	Vector3F vertices[] = {Vector3F(-1.f, -1.f, 0.f),Vector3F(1.f, -1.f, 0.f),Vector3F(-1.f, 1.f, 0.f),Vector3F(1.f, 1.f, 0.f)};

	vao.AddVertexBuffer(vertices, 4 * sizeof(Vector3F), { {ShaderDataType::Float3, "position"} });
	vao.AddIndexBuffer(elements, 6, IndicesType::UNSIGNED_SHORT);
}

void Plane::Draw(const Matrix4& Model, const Matrix4& View, const Matrix4& Projection, const GLuint shader)
{
	Matrix4F MVP = (Model*View*Projection).toFloat();

	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorValueHandle = glGetUniformLocation(shader, "MaterialColorValue");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorValueHandle, 1, &color.x);

	//bind vao before drawing
	vao.Bind();

	// Draw the triangles !
	vao.Draw();
}