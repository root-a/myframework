#include "Point.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>
#include "GraphicsStorage.h"

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
	vao.SetPrimitiveMode(PrimitiveMode::POINTS);
	SetUpBuffers();
}

Point::~Point()
{
}

void Point::SetUpBuffers()
{
	//we don't need that we can just assume in shader the origo point and multiply with transform or extract the position from mvp
	glm::vec3 vertex;
	BufferLayout vbVertex = { {ShaderDataType::Type::Float3, "position"} };
	vao.AddVertexBuffer(GraphicsStorage::assetRegistry.AllocAsset<VertexBuffer>(&vertex, (unsigned int)1, vbVertex));
}

void Point::Draw(const glm::mat4& Model, const glm::mat4& View, const glm::mat4& Projection, const GLuint shader, float size)
{
	glm::mat4 MVP = (Projection * View * Model);
	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorValueHandle = glGetUniformLocation(shader, "MaterialColorValue");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorValueHandle, 1, &color.x);

	vao.Bind();

	glPointSize(size);
	vao.Draw();
	glPointSize(1.f);
}