#include "Box.h"
#include "Material.h"
#include "Texture.h"
#include <algorithm>
#include <GL/glew.h>

Box * Box::Instance()
{
	static Box instance;

	return &instance;
}

Box::Box(){
	color.x = 1;
	color.y = 1;
	color.z = 0;
	SetUpBuffers();
}

Box::~Box()
{
}

void Box::SetUpBuffers()
{
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


	vao.vertexBuffers.reserve(2);

	vao.AddVertexBuffer(verts, 24 * sizeof(Vector3F), { {ShaderDataType::Float3, "position"} });
	vao.AddIndexBuffer(elements, 36, IndicesType::UNSIGNED_SHORT);
}

void Box::Draw(const Matrix4& ModelViewProjection, unsigned int shader)
{
	Matrix4F MVP = ModelViewProjection.toFloat();
	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorHandle = glGetUniformLocation(shader, "MaterialColor");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorHandle, 1, &color.x);

	tex->ActivateAndBind(0);

	vao.Bind();
	vao.Draw();
}