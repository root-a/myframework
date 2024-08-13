#include "BoundingBox.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>
#include "GraphicsStorage.h"
#include "Ebo.h"

BoundingBox * BoundingBox::Instance()
{
	static BoundingBox instance;

	return &instance;
}

BoundingBox::BoundingBox(){
	color.x = 1;
	color.y = 1;
	color.z = 0;
	vao.SetPrimitiveMode(PrimitiveMode::LINES);
	SetUpBuffers();
}

BoundingBox::~BoundingBox()
{
}

void BoundingBox::SetUpBuffers()
{
	glm::vec3 vertices[] = {
		glm::vec3(-0.5, -0.5, 0.5),
		glm::vec3(0.5, -0.5, 0.5),
		glm::vec3(0.5, 0.5, 0.5),
		glm::vec3(-0.5, 0.5, 0.5),
		   
		glm::vec3(-0.5, -0.5, -0.5),
		glm::vec3(0.5, -0.5, -0.5),
		glm::vec3(0.5, 0.5, -0.5),
		glm::vec3(-0.5, 0.5, -0.5)
	};

	GLubyte elements[] = {
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7
	};

	BufferLayout vbVertex({ {ShaderDataType::Type::Float3, "position"} });
	vao.AddVertexBuffer(GraphicsStorage::assetRegistry.AllocAsset<VertexBuffer>(vertices, (unsigned int)8, vbVertex));
	vao.AddElementBuffer(GraphicsStorage::assetRegistry.AllocAsset<ElementBuffer>(elements, (unsigned int)24));
}

void BoundingBox::Draw(const glm::mat4& Model, const glm::mat4& ProjectionView, unsigned int shader)
{
	glm::mat4 MVP = (ProjectionView * Model);
	MatrixHandle = glGetUniformLocation(shader, "MVP");
	MaterialColorValueHandle = glGetUniformLocation(shader, "MaterialColorValue");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(MaterialColorValueHandle, 1, &color.x);

	vao.Bind();
	vao.Draw();
}

