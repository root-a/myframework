#include "BoundingBoxSystem.h"
#include "Mesh.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>

using namespace mwm;

BoundingBoxSystem::BoundingBoxSystem(int maxCount){

	MaxCount = maxCount;
	LastUsed = 0;
	ActiveCount = 0;
	boundingBoxesContainer = new FastBoundingBox[maxCount];
	models = new Matrix4F[maxCount];
	colors = new Vector4[maxCount];
	SetUpBuffers();
}

BoundingBoxSystem::~BoundingBoxSystem()
{
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vaoHandle);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &colorBuffer);
	glDeleteBuffers(1, &modelBuffer);
	delete[] boundingBoxesContainer;
	delete[] models;
	delete[] colors;
}

const Vector3 BoundingBoxSystem::vertices[8] = {
	Vector3(-0.5, -0.5, 0.5),
	Vector3(0.5, -0.5, 0.5),
	Vector3(0.5, 0.5, 0.5),
	Vector3(-0.5, 0.5, 0.5),

	Vector3(-0.5, -0.5, -0.5),
	Vector3(0.5, -0.5, -0.5),
	Vector3(0.5, 0.5, -0.5),
	Vector3(-0.5, 0.5, -0.5)
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

int BoundingBoxSystem::UpdateContainer()
{
	int LinesCount = 0;
	for (int i = 0; i < MaxCount; i++){

		FastBoundingBox& bb = boundingBoxesContainer[i];

		if (bb.CanDraw())
		{
			models[LinesCount] = bb.node.TopDownTransform.toFloat();

			colors[LinesCount] = bb.color;

			LinesCount += 1;

			bb.UpdateDrawState();
		}
		else{
			bb.cameraDistance = -1.0f;
		}
	}
	return LinesCount;
}

void BoundingBoxSystem::SetUpBuffers()
{
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(Vector3), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 0); // position vertices : always reuse the same vertices -> 0

	glGenBuffers(1, &elementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(GLushort), elements, GL_STATIC_DRAW);

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector4), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1); // colors : one per box

	glGenBuffers(1, &modelBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Matrix4F), NULL, GL_STREAM_DRAW);
	for (unsigned int i = 0; i < 4; i++) {
		glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4F), (const GLvoid*)(sizeof(GLfloat) * i * 4));
		glEnableVertexAttribArray(2 + i);
		glVertexAttribDivisor(2 + i, 1); // model matrices : one per box
	}

	//Unbind the VAO now that the VBOs have been set up
	glBindVertexArray(0);
}

void BoundingBoxSystem::UpdateBuffers()
{
	//Bind VAO
	glBindVertexArray(vaoHandle);

	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Vector4), colors);

	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Matrix4F), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Matrix4F), models);
}

void BoundingBoxSystem::Draw(const mwm::Matrix4& ViewProjection, const unsigned int currentShaderID)
{
	UpdateBuffers();

	ViewProjectionHandle = glGetUniformLocation(currentShaderID, "VP");
	glUniformMatrix4fv(ViewProjectionHandle, 1, GL_FALSE, &ViewProjection.toFloat()[0][0]);

	glLineWidth(1.f);
	glDrawElementsInstanced(GL_LINES, 24, GL_UNSIGNED_SHORT, (void*)0, ActiveCount);

	glBindVertexArray(0);
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
	ActiveCount = UpdateContainer();
}

MinMax BoundingBoxSystem::CalcValuesInWorld(const Matrix3& modelM, const Vector3& position) const
{
	Vector3 maxValuesW = modelM * vertices[0];
	Vector3 minValuesW = modelM * vertices[0];
	Vector3 currentVertex;
	for (int i = 0; i < 8; ++i)
	{
		currentVertex = modelM * vertices[i];
		maxValuesW[0] = std::max(maxValuesW[0], currentVertex[0]);
		minValuesW[0] = std::min(minValuesW[0], currentVertex[0]);
		maxValuesW[1] = std::max(maxValuesW[1], currentVertex[1]);
		minValuesW[1] = std::min(minValuesW[1], currentVertex[1]);
		maxValuesW[2] = std::max(maxValuesW[2], currentVertex[2]);
		minValuesW[2] = std::min(minValuesW[2], currentVertex[2]);
	}
	MinMax mm;
	mm.max = maxValuesW + position;
	mm.min = minValuesW + position;
	return mm;
}