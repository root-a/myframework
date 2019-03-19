#include "Line.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>

using namespace mwm;

Line * Line::Instance()
{
	static Line instance;

	return &instance;
}

Line::Line()
{
	localMat = new Material();
	mat = localMat;
	SetUpBuffers();
}

Line::~Line()
{
	mat = nullptr;
	delete localMat;
}


void Line::SetUpBuffers()
{
	//Create VAO
	glGenVertexArrays(1, &vao.vaoHandle);
	//Bind VAO
	vao.Bind();

	Vector3F vertices[] = { Vector3F(0.f, 0.f, 0.f),Vector3F(0.f, 0.f, 1.f) };
		
	GLuint vertexbuffer;
	// 1rst attribute buffer : vertices
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(Vector3F), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
	glEnableVertexAttribArray(0);
	vao.vertexBuffers.push_back(vertexbuffer);

	//Unbind the VAO now that the VBOs have been set up
	vao.Unbind();
}

void Line::Draw(const Matrix4& Model, const Matrix4& View, const Matrix4& Projection, const GLuint shader, float width)
{
	Matrix4F MVP = (Model*View*Projection).toFloat();
	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorValueHandle = glGetUniformLocation(shader, "MaterialColorValue");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorValueHandle, 1, &mat->color.x);

	//bind vao before drawing
	vao.Bind();

	glLineWidth(width);
	// Draw the lines !
	glDrawArrays(GL_LINES, 0, 2);
	glLineWidth(1.f);
}