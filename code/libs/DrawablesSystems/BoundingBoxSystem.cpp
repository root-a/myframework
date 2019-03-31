#include "BoundingBoxSystem.h"
#include "Material.h"
#include "Object.h"
#include <algorithm>
#include <GL/glew.h>

using namespace mwm;

BoundingBoxSystem::BoundingBoxSystem(int maxCount){
	MaxCount = maxCount;
	LastUsed = 0;
	ActiveCount = 0;
	boundingBoxesContainer = new FastBoundingBox[maxCount];
	models = new Matrix4F[maxCount];
	colors = new Vector3F[maxCount];
	SetUpBuffers();
	paused = false;
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
	
	vao.Bind();

	vao.vertexBuffers.reserve(4);

	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(Vector3F), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 0); // position vertices : always reuse the same vertices -> 0
	vao.vertexBuffers.push_back(vertexBuffer);
	
	vao.indicesCount = 24;
	GLuint elementBuffer;
	glGenBuffers(1, &elementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vao.indicesCount * sizeof(GLushort), elements, GL_STATIC_DRAW);
	vao.vertexBuffers.push_back(elementBuffer);

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector3F), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1); // colors : one per box
	vao.vertexBuffers.push_back(colorBuffer);

	glGenBuffers(1, &modelBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Matrix4F), NULL, GL_STREAM_DRAW);
	for (unsigned int i = 0; i < 4; i++) {
		glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4F), (const GLvoid*)(sizeof(GLfloat) * i * 4));
		glEnableVertexAttribArray(2 + i);
		glVertexAttribDivisor(2 + i, 1); // model matrices : one per box
	}
	vao.vertexBuffers.push_back(modelBuffer);

	//Unbind the VAO now that the VBOs have been set up
	vao.Unbind();
}

void BoundingBoxSystem::UpdateBuffers()
{
	glBindVertexArray(vao.vaoHandle);

	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Vector3F), colors);

	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Matrix4F), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Matrix4F), models);
}

int BoundingBoxSystem::Draw(const mwm::Matrix4& ViewProjection, const unsigned int currentShaderID)
{
	UpdateContainer();
	UpdateBuffers();

	vao.Bind();

	ViewProjectionHandle = glGetUniformLocation(currentShaderID, "VP");
	glUniformMatrix4fv(ViewProjectionHandle, 1, GL_FALSE, &ViewProjection.toFloat()[0][0]);

	glLineWidth(1.f);
	glDrawElementsInstanced(GL_LINES, vao.indicesCount, GL_UNSIGNED_SHORT, (void*)0, ActiveCount);

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