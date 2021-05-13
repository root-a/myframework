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
	///GLuint bindingIndex = 0;
	///GLuint attributeIndex = 0;
	///
	///GLuint vertexbuffer;
	///glCreateBuffers(1, &vertexbuffer);
	///glNamedBufferStorage(vertexbuffer, 4 * sizeof(Vector3F), vertices, GL_DYNAMIC_STORAGE_BIT);
	///glEnableVertexArrayAttrib(vao.handle, attributeIndex); //vao handle, attribute index, which attrib index to enable on this vao
	///glVertexArrayVertexBuffer(vao.handle, bindingIndex, vertexbuffer, 0, sizeof(Vector3F)); //vao handle, binding index, vbo handle, offset to first element, stride (distance between elements)
	///glVertexArrayAttribFormat(vao.handle, attributeIndex, 3, GL_FLOAT, GL_FALSE, 0); //vao handle, attribute index, values per element, type of data, normalized, relativeoffset - The distance between elements within the buffer.
	///glVertexArrayAttribBinding(vao.handle, attributeIndex, bindingIndex); //vao handle, attribute index, binding index
	///vao.vertexBuffers.push_back(vertexbuffer);

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