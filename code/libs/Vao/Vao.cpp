#include "Vao.h"
#include <GL/glew.h>

Vao::Vao()
{
}

Vao::~Vao()
{
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vaoHandle);
	glDeleteVertexArrays(vertexBuffers.size(), &vertexBuffers[0]);
}

void Vao::Bind()
{
	glBindVertexArray(vaoHandle);
}

void Vao::Unbind()
{
	glBindVertexArray(0);
}
