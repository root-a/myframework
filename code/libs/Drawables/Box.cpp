#include "Box.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include <algorithm>
#include <GL/glew.h>

using namespace mwm;

Box::Box(){
	mat = new Material();
	SetUpBuffers();
}

const Vector3F Box::verts[24] = {
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

const GLushort Box::elements[] = {
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



void Box::SetUpBuffers()
{
	mesh = new Mesh();

	glGenVertexArrays(1, &mesh->vaoHandle);
	glBindVertexArray(mesh->vaoHandle);

	glGenBuffers(1, &mesh->vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(Vector3F), &verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &mesh->elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLushort), &elements[0], GL_STATIC_DRAW);
	mesh->indicesSize = 36;

	glBindVertexArray(0);

}

void Box::Draw(const mwm::Matrix4& ModelViewProjection, unsigned int wireframeShader)
{
	Matrix4F MVP = ModelViewProjection.toFloat();
	MatrixHandle = glGetUniformLocation(wireframeShader, "MVP");
	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(mat->texture->target, mat->texture->handle);

	glBindVertexArray(this->mesh->vaoHandle);
	glDrawElements(GL_TRIANGLES, mesh->indicesSize, GL_UNSIGNED_SHORT, 0);
}