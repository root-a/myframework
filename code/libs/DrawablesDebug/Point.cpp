#include "Point.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>
using namespace mwm;

Point* Point::Instance()
{
	static Point instance;

	return &instance;
}

Point::Point()
{
	localMat = new Material();
	mat = localMat;
	SetUpBuffers();
}

Point::~Point()
{
	mat = nullptr;
	delete localMat;
}

void Point::SetUpBuffers()
{
	//Create VAO
	glGenVertexArrays(1, &vao.vaoHandle);
	//Bind VAO
	vao.Bind();

	GLuint vertexbuffer;
	// 1rst attribute buffer : vertices
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 1 * sizeof(Vector3F), &Vector3F()[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
	glEnableVertexAttribArray(0);
	vao.vertexBuffers.push_back(vertexbuffer);

	//Unbind the VAO now that the VBOs have been set up
	vao.Unbind();
}

void Point::Draw(const Matrix4& Model, const Matrix4& View, const Matrix4& Projection, const GLuint shader, float size)
{
	Matrix4F MVP = (Model*View*Projection).toFloat();
	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorValueHandle = glGetUniformLocation(shader, "MaterialColorValue");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorValueHandle, 1, &mat->color.x);

	//bind vao before drawing
	vao.Bind();

	glPointSize(size);
	// Draw the lines !
	glDrawElements(GL_POINTS, 1, GL_UNSIGNED_SHORT, 0);
	glPointSize(1.f);
}