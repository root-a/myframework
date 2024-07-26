#include "PointSystem.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>
#include "GraphicsStorage.h"

PointSystem::PointSystem(int maxCount){

	MaxCount = maxCount;
	LastUsed = 0;
	pointsContainer = new FastPoint[maxCount];
	vao.SetPrimitiveMode(PrimitiveMode::POINTS);
	SetUpBuffers();
}

PointSystem::~PointSystem()
{
	delete[] pointsContainer;
}

int PointSystem::FindUnused()
{
	for (int i = LastUsed; i < MaxCount; i++){
		if (pointsContainer[i].CanDraw() == false){
			LastUsed = i;
			return i;
		}
	}

	for (int i = 0; i < LastUsed; i++){
		if (pointsContainer[i].CanDraw() == false){
			LastUsed = i;
			return i;
		}
	}

	return 0;
}

void PointSystem::UpdateContainer()
{
	positionColorBuffer->activeCount = 0;

	for (int i = 0; i < MaxCount; i++){

		FastPoint& p = pointsContainer[i];

		if (p.CanDraw())
		{
			positionColorBuffer->SetElementData(positionColorBuffer->activeCount, &p.data);
			positionColorBuffer->IncreaseInstanceCount();

			p.UpdateDrawState();
		}
	}
}

void PointSystem::SetUpBuffers()
{
	BufferLayout vbPositionColor({ {ShaderDataType::Type::Float3, "Position"}, {ShaderDataType::Type::Float4, "Color"} });
	positionColorBuffer = GraphicsStorage::assetRegistry.AllocAsset<VertexBufferDynamic>(nullptr, MaxCount, vbPositionColor);
	vao.AddVertexBuffer(positionColorBuffer);

}

void PointSystem::UpdateBuffers()
{
	positionColorBuffer->Update();
}


void PointSystem::Draw(const glm::mat4& ViewProjection, const unsigned int currentShaderID, float size)
{
	if (dirty)
	{
		UpdateContainer();
		UpdateBuffers();
		dirty = false;
	}
	vao.Bind();

	glPointSize(size);
	vao.activeCount = positionColorBuffer->activeCount;
	vao.Draw();
	glPointSize(1.f);
}

FastPoint* PointSystem::GetPoint()
{
	FastPoint* fp = &pointsContainer[FindUnused()];
	fp->DrawAlways();
	dirty = true;
	return fp;
}

FastPoint* PointSystem::GetPointOnce()
{
	FastPoint* fp = &pointsContainer[FindUnused()];
	fp->DrawOnce();
	dirty = true;
	return fp;
}

void PointSystem::Update()
{
	if (!paused)
	{
		UpdateContainer();
		UpdateBuffers();
	}
}

Component* PointSystem::Clone()
{
	return new PointSystem(*this);
}
