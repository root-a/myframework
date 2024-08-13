#include "Line.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>
#include "GraphicsStorage.h"

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
	vao.SetPrimitiveMode(PrimitiveMode::LINES);
	SetUpBuffers();
}

Line::~Line()
{
}


void Line::SetUpBuffers()
{
	glm::vec3 vertices[] = {
		glm::vec3(0.f, 0.f, 0.f),
		glm::vec3(0.f, 0.f, 1.f)
	};

	BufferLayout vbVertex = { {ShaderDataType::Type::Float3, "position"} };
	vao.AddVertexBuffer(GraphicsStorage::assetRegistry.AllocAsset<VertexBuffer>(vertices, (unsigned int)2, vbVertex));
	vao.activeCount = 2;
}

void Line::Draw(const glm::mat4& Model, const glm::mat4& View, const glm::mat4& Projection, const GLuint shader)
{
	glm::mat4 MVP = (Projection * View * Model);
	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorValueHandle = glGetUniformLocation(shader, "MaterialColorValue");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorValueHandle, 1, &color.x);

	vao.Bind();
	vao.Draw();
}