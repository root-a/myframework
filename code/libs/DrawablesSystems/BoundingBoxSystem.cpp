#include "BoundingBoxSystem.h"
#include "Material.h"
#include "Object.h"
#include <algorithm>
#include <GL/glew.h>



BoundingBoxSystem::BoundingBoxSystem(int maxCount){
	MaxCount = maxCount;
	LastUsed = 0;
	ActiveCount = 0;
	boundingBoxesContainer = new FastBoundingBox[maxCount];
	models = new Matrix4F[maxCount];
	colors = new Vector3F[maxCount];
	SetUpBuffers();
	paused = false;
	vao.SetPrimitiveMode(Vao::PrimitiveMode::LINES);
}

BoundingBoxSystem::~BoundingBoxSystem()
{
	delete[] boundingBoxesContainer;
	delete[] models;
	delete[] colors;
}

const Vector3F BoundingBoxSystem::vertices[8] = {
	Vector3F(-0.5f, -0.5f, 0.5f),
	Vector3F(0.5f, -0.5f, 0.5f),
	Vector3F(0.5f, 0.5f, 0.5f),
	Vector3F(-0.5f, 0.5f, 0.5f),

	Vector3F(-0.5f, -0.5f, -0.5f),
	Vector3F(0.5f, -0.5f, -0.5f),
	Vector3F(0.5f, 0.5f, -0.5f),
	Vector3F(-0.5f, 0.5f, -0.5f)
};

const GLushort BoundingBoxSystem::elements[] = {
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
	ActiveCount = 0;
	for (int i = 0; i < MaxCount; i++){

		FastBoundingBox& bb = boundingBoxesContainer[i];

		if (bb.CanDraw())
		{
			models[ActiveCount] = bb.model->toFloat();

			colors[ActiveCount] = *bb.color;

			ActiveCount += 1;

			bb.UpdateDrawState();
		}
	}
}

void BoundingBoxSystem::SetUpBuffers()
{
	vao.vertexBuffers.reserve(4);
	vao.AddVertexBuffer(vertices, 8 * sizeof(Vector3F), { {ShaderDataType::Float3, "Position"} });
	vao.AddIndexBuffer(elements, 24, IndicesType::UNSIGNED_SHORT);
	colorBuffer = vao.AddVertexBuffer(NULL, MaxCount * sizeof(Vector3F), { {ShaderDataType::Float3, "Color", 1} });
	modelBuffer = vao.AddVertexBuffer(NULL, MaxCount * sizeof(Matrix4F), { {ShaderDataType::Mat4, "M", 1} });
}

void BoundingBoxSystem::UpdateBuffers()
{
	glNamedBufferSubData(modelBuffer, 0, ActiveCount * sizeof(Matrix4F), models);
	glNamedBufferSubData(colorBuffer, 0, ActiveCount * sizeof(Vector3F), colors);
}

int BoundingBoxSystem::Draw(const Matrix4& ViewProjection, const unsigned int currentShaderID)
{
	UpdateContainer();
	UpdateBuffers();

	vao.Bind();

	glLineWidth(1.f);
	vao.activeCount = ActiveCount;
	vao.Draw();

	return ActiveCount;
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