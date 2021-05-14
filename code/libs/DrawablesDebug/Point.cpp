#include "Point.h"
#include <algorithm>
#include <GL/glew.h>

Point* Point::Instance()
{
	static Point instance;

	return &instance;
}

Point::Point()
{
	color.x = 1;
	color.y = 1;
	color.z = 0;
	vao.SetPrimitiveMode(Vao::PrimitiveMode::POINTS);
	SetUpBuffers();
}

Point::~Point()
{
}

void Point::SetUpBuffers()
{
	vao.AddVertexBuffer(&Vector3F()[0], 1 * sizeof(Vector3F), { {ShaderDataType::Float3, "position"} });
}

void Point::Draw(const Matrix4& Model, const Matrix4& View, const Matrix4& Projection, const GLuint shader, float size)
{
	Matrix4F MVP = (Model*View*Projection).toFloat();
	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorValueHandle = glGetUniformLocation(shader, "MaterialColorValue");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorValueHandle, 1, &color.x);

	vao.Bind();

	glPointSize(size);
	vao.Draw();
	glPointSize(1.f);
}