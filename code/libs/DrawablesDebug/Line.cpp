#include "Line.h"
#include <algorithm>
#include <GL/glew.h>

Line * Line::Instance()
{
	static Line instance;

	return &instance;
}

Line::Line()
{
	color.x = 1;
	color.y = 1;
	color.z = 0;
	vao.SetPrimitiveMode(Vao::PrimitiveMode::LINES);
	SetUpBuffers();
}

Line::~Line()
{
}


void Line::SetUpBuffers()
{
	Vector3F vertices[] = { Vector3F(0.f, 0.f, 0.f),Vector3F(0.f, 0.f, 1.f) };
	
	vao.AddVertexBuffer(vertices, 2 * sizeof(Vector3F), { {ShaderDataType::Float3, "position"} });
	vao.activeCount = 2;
}

void Line::Draw(const Matrix4& Model, const Matrix4& View, const Matrix4& Projection, const GLuint shader)
{
	Matrix4F MVP = (Model*View*Projection).toFloat();
	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorValueHandle = glGetUniformLocation(shader, "MaterialColorValue");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorValueHandle, 1, &color.x);

	//bind vao before drawing
	vao.Bind();
	//draw the lines!
	vao.Draw();
}