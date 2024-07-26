#include "CircleSystem.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>
#include "CameraManager.h"
#include <GraphicsStorage.h>
#include "Ebo.h"

// goal is to remove any OBJ and VAO code from here
//OBJ
//vertices are mathematically clockwise
const glm::vec3 CircleSystem::g_vertex_buffer_data[4] = {
	glm::vec3(-1.0f, -1.0f, 0.0f),
	glm::vec3(-1.0f, 1.0f, 0.0f),
	glm::vec3(1.0f, 1.0f, 0.0f),
	glm::vec3(1.0f, -1.0f, 0.0f),
};

//OBJ
//indices are mathematically counter-clockwise
const GLubyte CircleSystem::elements[] = {
	0, 3, 2,
	2, 1, 0
};

CircleSystem::CircleSystem(int maxCount){

	MaxCount = maxCount;
	LastUsed = 0;
	circlesContainer = new FastCircle[maxCount];
	vao.SetPrimitiveMode(PrimitiveMode::TRIANGLES);
	SetUpBuffers();
}

CircleSystem::~CircleSystem()
{
	delete[] circlesContainer;
}

int CircleSystem::FindUnused()
{
	for (int i = LastUsed; i < MaxCount; i++){
		if (circlesContainer[i].CanDraw() == false){
			LastUsed = i;
			return i;
		}
	}

	for (int i = 0; i < LastUsed; i++){
		if (circlesContainer[i].CanDraw() == false){
			LastUsed = i;
			return i;
		}
	}

	return 0;
}

void CircleSystem::UpdateContainer()
{
	glm::vec3 camPos = CameraManager::Instance()->cameraPos;
	for (int i = 0; i < MaxCount; i++)
	{
		FastCircle& fp = circlesContainer[i];
		fp.cameraDistance = glm::length(fp.data.center - camPos);
	}
	std::sort(&circlesContainer[0], &circlesContainer[MaxCount - 1]);

	positionsSizesColorsSoftnessThicknessBuffer->activeCount = 0;

	for (int i = 0; i < MaxCount; i++){

		FastCircle& p = circlesContainer[i];

		if (p.CanDraw())
		{
			positionsSizesColorsSoftnessThicknessBuffer->SetElementData(positionsSizesColorsSoftnessThicknessBuffer->activeCount, &p.data);

			positionsSizesColorsSoftnessThicknessBuffer->IncreaseInstanceCount();

			p.UpdateDrawState();
		}
	}
}

void CircleSystem::SetUpBuffers()
{
	BufferLayout vbVertex({ {ShaderDataType::Type::Float3, "Position"} });
	BufferLayout vbCenterRadiusColorSoftnessThickness({ {ShaderDataType::Type::Float3, "CenterPosition", 1}, {ShaderDataType::Type::Float, "Radius", 1}, {ShaderDataType::Type::Float4, "Color", 1}, {ShaderDataType::Type::Float, "Softness", 1}, {ShaderDataType::Type::Float, "Thickness", 1} });
	positionsSizesColorsSoftnessThicknessBuffer = GraphicsStorage::assetRegistry.AllocAsset<VertexBufferDynamic>(nullptr, MaxCount, vbCenterRadiusColorSoftnessThickness);
	vao.AddVertexBuffer(GraphicsStorage::assetRegistry.AllocAsset<VertexBuffer>(g_vertex_buffer_data, (unsigned int)4, vbVertex));
	vao.AddVertexBuffer(positionsSizesColorsSoftnessThicknessBuffer);
	vao.AddElementBuffer(GraphicsStorage::assetRegistry.AllocAsset<ElementBuffer>(elements, (unsigned int)6));

}

void CircleSystem::UpdateBuffers()
{
	positionsSizesColorsSoftnessThicknessBuffer->Update();
}


void CircleSystem::Draw(const glm::mat4& ViewProjection, const unsigned int currentShaderID, float size)
{
	if (dirty)
	{
		UpdateContainer();
		UpdateBuffers();
		dirty = false;
	}
	vao.Bind();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	vao.activeCount = positionsSizesColorsSoftnessThicknessBuffer->activeCount;
	vao.Draw();
	glDisable(GL_BLEND);
}

FastCircle* CircleSystem::GetCircle()
{
	FastCircle* fp = &circlesContainer[FindUnused()];
	fp->DrawAlways();
	dirty = true;
	return fp;
}

FastCircle* CircleSystem::GetCircleOnce()
{
	FastCircle* fp = &circlesContainer[FindUnused()];
	fp->DrawOnce();
	dirty = true;
	return fp;
}

void CircleSystem::Update()
{
	if (!paused)
	{
		UpdateContainer();
		UpdateBuffers();
	}
}

Component* CircleSystem::Clone()
{
	return new CircleSystem(*this);
}
