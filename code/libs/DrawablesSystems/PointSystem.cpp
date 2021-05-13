#include "PointSystem.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>

PointSystem::PointSystem(int maxCount){

	MaxCount = maxCount;
	LastUsed = 0;
	ActiveCount = 0;
	pointsContainer = new FastPoint[maxCount];
	positions = new Vector3F[maxCount];
	colors = new Vector4F[maxCount];
	vao.SetPrimitiveMode(Vao::PrimitiveMode::POINTS);
	SetUpBuffers();
}

const Vector3F PointSystem::vertices[] = {
	Vector3F(0.f, 0.f, 0.f)
};

PointSystem::~PointSystem()
{
	delete[] pointsContainer;
	delete[] positions;
	delete[] colors;
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
	ActiveCount = 0;
	for (int i = 0; i < MaxCount; i++){

		FastPoint& p = pointsContainer[i];

		if (p.CanDraw())
		{
			positions[ActiveCount] = p.node.localPosition.toFloat();
			colors[ActiveCount] = p.color;

			ActiveCount += 1;

			p.UpdateDrawState();
		}
	}
}

void PointSystem::SetUpBuffers()
{
	vertexBuffer = vao.AddVertexBuffer(NULL, MaxCount * sizeof(Vector3F), { {ShaderDataType::Float3, "Position"} });
	colorBuffer = vao.AddVertexBuffer(NULL, MaxCount * sizeof(Vector4F), { {ShaderDataType::Float4, "Color"} });
}

void PointSystem::UpdateBuffers()
{
	glNamedBufferSubData(vertexBuffer, 0, ActiveCount * sizeof(Vector3F), positions);
	glNamedBufferSubData(colorBuffer, 0, ActiveCount * sizeof(Vector4F), colors);
}


void PointSystem::Draw(const Matrix4& ViewProjection, const unsigned int currentShaderID, float size)
{
	if (dirty)
	{
		UpdateContainer();
		UpdateBuffers();
		dirty = false;
	}
	vao.Bind();

	//ViewProjectionHandle = glGetUniformLocation(currentShaderID, "VP");
	//glUniformMatrix4fv(ViewProjectionHandle, 1, GL_FALSE, &ViewProjection.toFloat()[0][0]);

	glPointSize(size);
	vao.activeCount = ActiveCount;
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