#include "BoundingBox.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>



BoundingBox * BoundingBox::Instance()
{
	static BoundingBox instance;

	return &instance;
}

BoundingBox::BoundingBox(){
	color.x = 1;
	color.y = 1;
	color.z = 0;
	vao.SetPrimitiveMode(Vao::PrimitiveMode::LINES);
	SetUpBuffers();
}

BoundingBox::~BoundingBox()
{
}

void BoundingBox::SetUpBuffers()
{
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

	vao.AddVertexBuffer(verts, 8 * sizeof(Vector3F), { {ShaderDataType::Float3, "position"} });
	vao.AddIndexBuffer(elements, 24, IndicesType::UNSIGNED_SHORT);
}

void BoundingBox::Draw(const Matrix4& Model, const Matrix4& ViewProjection, unsigned int shader)
{
	Matrix4F MVP = (Model*ViewProjection).toFloat();
	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorValueHandle = glGetUniformLocation(shader, "MaterialColorValue");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorValueHandle, 1, &color.x);

	vao.Bind();
	vao.Draw();
}

