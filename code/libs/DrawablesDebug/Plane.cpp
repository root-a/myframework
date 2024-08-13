#include "Plane.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>
#include "GraphicsStorage.h"
#include "Ebo.h"

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
	unsigned char elements[] = { 0, 1, 2, 2, 1, 3 };
	glm::vec3 vertices[] = { 
		glm::vec3(-1.f, -1.f, 0.f),
		glm::vec3(1.f, -1.f, 0.f),
		glm::vec3(-1.f, 1.f, 0.f),
		glm::vec3(1.f, 1.f, 0.f)
	};

	BufferLayout vbVertex = { {ShaderDataType::Type::Float3, "position"} };
	vao.AddVertexBuffer(GraphicsStorage::assetRegistry.AllocAsset<VertexBuffer>(vertices, (unsigned int)4, vbVertex));
	vao.AddElementBuffer(GraphicsStorage::assetRegistry.AllocAsset<ElementBuffer>(elements, (unsigned int)6));
}

void Plane::Draw(const glm::mat4& Model, const glm::mat4& View, const glm::mat4& Projection, const GLuint shader)
{
	glm::mat4 MVP = (Projection * View * Model);

	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorValueHandle = glGetUniformLocation(shader, "MaterialColorValue");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorValueHandle, 1, &color.x);

	vao.Bind();
	vao.Draw();
}