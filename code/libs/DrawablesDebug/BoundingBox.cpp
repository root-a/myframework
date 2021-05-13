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

	///GLuint bindingIndex = 0;
	///GLuint attributeIndex = 0;
	///
	///GLuint vertexbuffer;
	///glCreateBuffers(1, &vertexbuffer);
	///glNamedBufferStorage(vertexbuffer, 8 * sizeof(Vector3F), verts, GL_DYNAMIC_STORAGE_BIT);
	///glEnableVertexArrayAttrib(vao.handle, attributeIndex); //vao handle, attribute index, which attrib index to enable on this vao
	///glVertexArrayVertexBuffer(vao.handle, bindingIndex, vertexbuffer, 0, sizeof(Vector3F)); //vao handle, binding index, vbo handle, offset to first element, stride (distance between elements)
	///glVertexArrayAttribFormat(vao.handle, attributeIndex, 3, GL_FLOAT, GL_FALSE, 0); //vao handle, attribute index, values per element, type of data, normalized, relativeoffset - The distance between elements within the buffer.
	///glVertexArrayAttribBinding(vao.handle, attributeIndex, bindingIndex); //vao handle, attribute index, binding index
	///vao.vertexBuffers.push_back(vertexbuffer);

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

