#include "BoundingBoxSystem.h"
#include "Material.h"
#include "Object.h"
#include <algorithm>
#include <GL/glew.h>
#include <GraphicsStorage.h>
#include "Ebo.h"

BoundingBoxSystem::BoundingBoxSystem(int maxCount){
	MaxCount = maxCount;
	LastUsed = 0;
	boundingBoxesContainer = new FastBoundingBox[maxCount];
	SetUpBuffers();
	paused = false;
	vao.SetPrimitiveMode(PrimitiveMode::LINES);
}

BoundingBoxSystem::~BoundingBoxSystem()
{
	delete[] boundingBoxesContainer;
}

const glm::vec3 BoundingBoxSystem::vertices[8] = {
	glm::vec3(-0.5f, -0.5f, 0.5f),
	glm::vec3(0.5f, -0.5f, 0.5f),
	glm::vec3(0.5f, 0.5f, 0.5f),
	glm::vec3(-0.5f, 0.5f, 0.5f),

	glm::vec3(-0.5f, -0.5f, -0.5f),
	glm::vec3(0.5f, -0.5f, -0.5f),
	glm::vec3(0.5f, 0.5f, -0.5f),
	glm::vec3(-0.5f, 0.5f, -0.5f)
};

const GLubyte BoundingBoxSystem::elements[] = {
	0, 1, 1, 2, 2, 3, 3, 0,
	4, 5, 5, 6, 6, 7, 7, 4,
	0, 4, 1, 5, 2, 6, 3, 7
};

int BoundingBoxSystem::FindUnused()
{
	for (int i = LastUsed; i < MaxCount; i++){
		if (boundingBoxesContainer[i].CanDraw() == false){
			LastUsed = i;
			return i;
		}
	}

	for (int i = 0; i < LastUsed; i++){
		if (boundingBoxesContainer[i].CanDraw() == false){
			LastUsed = i;
			return i;
		}
	}

	return 0;
}

void BoundingBoxSystem::UpdateContainer()
{
	colorModelBuffer->activeCount = 0;
	for (int i = 0; i < MaxCount; i++){

		FastBoundingBox& bb = boundingBoxesContainer[i];

		if (bb.CanDraw())
		{
			colorModelBuffer->SetElementData(colorModelBuffer->activeCount, &bb.data);
			colorModelBuffer->IncreaseInstanceCount();
			bb.UpdateDrawState();
		}
	}
}

void BoundingBoxSystem::SetUpBuffers()
{
	BufferLayout vbVertex({ {ShaderDataType::Type::Float3, "Position"} });
	BufferLayout vbColorModel({ {ShaderDataType::Type::Float3, "Color", 1}, {ShaderDataType::Type::FloatMat4, "M", 1} });
	colorModelBuffer = GraphicsStorage::assetRegistry.AllocAsset<VertexBufferDynamic>(nullptr, MaxCount, vbColorModel);
	vao.AddVertexBuffer(GraphicsStorage::assetRegistry.AllocAsset<VertexBuffer>(vertices, (unsigned int)8, vbVertex));
	vao.AddVertexBuffer(colorModelBuffer);
	vao.AddElementBuffer(GraphicsStorage::assetRegistry.AllocAsset<ElementBuffer>(elements, (unsigned int)24));
}

void BoundingBoxSystem::UpdateBuffers()
{
	colorModelBuffer->Update();
}

int BoundingBoxSystem::Draw(const glm::mat4& ViewProjection, const unsigned int currentShaderID)
{
	UpdateContainer();
	UpdateBuffers();

	vao.Bind();

	glLineWidth(1.f);
	vao.activeCount = colorModelBuffer->activeCount;
	vao.Draw();

	return colorModelBuffer->activeCount;
}

FastBoundingBox* BoundingBoxSystem::GetBoundingBox()
{
	FastBoundingBox* bb = &boundingBoxesContainer[FindUnused()];
	bb->DrawAlways();
	return bb;
}

FastBoundingBox* BoundingBoxSystem::GetBoundingBoxOnce()
{
	FastBoundingBox* bb = &boundingBoxesContainer[FindUnused()];
	bb->DrawOnce();
	return bb;
}

void BoundingBoxSystem::Update()
{
}

Component* BoundingBoxSystem::Clone()
{
	return new BoundingBoxSystem(*this);
}
