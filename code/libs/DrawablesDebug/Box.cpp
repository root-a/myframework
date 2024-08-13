#include "Box.h"
#include "Material.h"
#include "Texture.h"
#include <algorithm>
#include <GL/glew.h>
#include "GraphicsStorage.h"
#include "Ebo.h"

Box * Box::Instance()
{
	static Box instance;

	return &instance;
}

Box::Box(){
	color.x = 1;
	color.y = 1;
	color.z = 0;
	SetUpBuffers();
}

Box::~Box()
{
}

void Box::SetUpBuffers()
{
	glm::vec3 vertices[] = {
		glm::vec3(-1.0f, -1.0f,  1.0f),
		glm::vec3(-1.0f,  1.0f,  1.0f),
		glm::vec3(1.0f, -1.0f,  1.0f),
		glm::vec3(1.0f,  1.0f,  1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f,  1.0f, -1.0f),
		glm::vec3(1.0f, -1.0f, -1.0f),
		glm::vec3(1.0f,  1.0f, -1.0f)
	};
	
	GLubyte elements[] = {
		0, 1, 2, 1, 3, 2,
		1, 5, 3, 5, 7, 3,
		5, 4, 7, 4, 6, 7,
		4, 0, 6, 0, 2, 6,
		2, 3, 6, 3, 7, 6,
		4, 5, 0, 5, 1, 0
	};

	BufferLayout vbVertex({ {ShaderDataType::Type::Float3, "position"} });
	vao.AddVertexBuffer(GraphicsStorage::assetRegistry.AllocAsset<VertexBuffer>(vertices, (unsigned int)24, vbVertex));
	vao.AddElementBuffer(GraphicsStorage::assetRegistry.AllocAsset<ElementBuffer>(elements, (unsigned int)36));
}

void Box::Draw(const glm::mat4& ModelViewProjection, unsigned int shader)
{
	glm::mat4 MVP = ModelViewProjection;
	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorHandle = glGetUniformLocation(shader, "MaterialColor");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorHandle, 1, &color.x);

	tex->ActivateAndBind(0);

	vao.Bind();
	vao.Draw();
}