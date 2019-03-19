#include "PointSystem.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>
using namespace mwm;
PointSystem::PointSystem(int maxCount){

	MaxCount = maxCount;
	LastUsed = 0;
	ActiveCount = 0;
	pointsContainer = new FastPoint[maxCount];
	positions = new Vector3F[maxCount];
	colors = new Vector4F[maxCount];
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
	glGenVertexArrays(1, &vao.vaoHandle);
	vao.Bind();

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector3F), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	vao.vertexBuffers.push_back(vertexBuffer);

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector4F), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);
	vao.vertexBuffers.push_back(colorBuffer);

	vao.Unbind();
}

void PointSystem::UpdateBuffers()
{
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Vector3F), positions);

	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Vector4F), colors);
}


void PointSystem::Draw(const mwm::Matrix4& ViewProjection, const unsigned int currentShaderID, float size)
{
	if (dirty)
	{
		UpdateContainer();
		UpdateBuffers();
		dirty = false;
	}
	vao.Bind();

	ViewProjectionHandle = glGetUniformLocation(currentShaderID, "VP");
	glUniformMatrix4fv(ViewProjectionHandle, 1, GL_FALSE, &ViewProjection.toFloat()[0][0]);

	glPointSize(size);
	glDrawArrays(GL_POINTS, 0, ActiveCount);
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