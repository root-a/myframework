#include "Line.h"
#include "Mesh.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>

using namespace mwm;
Line::Line(){

	mat = new Material();
	SetUpBuffers();
}

const Vector3 Line::vertices[] = {
	Vector3(0.f, 0.f, 0.f),
	Vector3(0.f, 0.f, 1.f)
};


void Line::SetUpBuffers()
{
	mesh = new Mesh();
	//Create VAO
	glGenVertexArrays(1, &mesh->vaoHandle);
	//Bind VAO
	glBindVertexArray(mesh->vaoHandle);

	// 1rst attribute buffer : vertices
	glGenBuffers(1, &mesh->vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(Vector3), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
	glEnableVertexAttribArray(0);

	//Unbind the VAO now that the VBOs have been set up
	glBindVertexArray(0);
}

void Line::Draw(const Matrix4& Model, const Matrix4& View, const Matrix4& Projection, const GLuint shader, float width)
{
	Matrix4F MVP = (Model*View*Projection).toFloat();
	//GLuint prevShader = ShaderManager::Instance()->GetCurrentShaderID();
	//GLuint wireframeShader = ShaderManager::Instance()->shaderIDs["wireframe"];
	//glUseProgram(wireframeShader);
	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorValueHandle = glGetUniformLocation(shader, "MaterialColorValue");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorValueHandle, 1, &this->mat->color.vect[0]);

	//bind vao before drawing
	glBindVertexArray(this->mesh->vaoHandle);

	glLineWidth(width);
	// Draw the lines !
	glDrawArrays(GL_LINES, 0, 2);
	glLineWidth(1.f);
	//glUseProgram(prevShader);
}