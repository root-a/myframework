#include "PointSystem.h"
#include "Mesh.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>
using namespace mwm;
PointSystem::PointSystem(int maxCount){

	MaxCount = maxCount;
	LastUsed = 0;
	ActiveCount = 0;
	pointsContainer = new FastPoint[maxCount];
	positions = new Vector3[maxCount];
	colors = new Vector4[maxCount];
	SetUpBuffers();
}

const Vector3 PointSystem::vertices[] = {
	Vector3(0.f, 0.f, 0.f)
};

PointSystem::~PointSystem()
{
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vaoHandle);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &colorBuffer);
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

int PointSystem::UpdateContainer()
{
	int LinesCount = 0;
	for (int i = 0; i < MaxCount; i++){

		FastPoint& p = pointsContainer[i];

		if (p.CanDraw())
		{
			positions[LinesCount] = p.node.position;

			colors[LinesCount] = p.color;

			LinesCount += 1;

			p.UpdateDrawState();
		}
		else{
			p.cameraDistance = -1.0f;
		}
	}
	return LinesCount;
}

void PointSystem::SetUpBuffers()
{
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector3), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector4), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void PointSystem::UpdateBuffers()
{
	//Bind VAO
	glBindVertexArray(vaoHandle);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Vector3), positions);

	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Vector4), colors);
}


void PointSystem::Draw(const mwm::Matrix4& ViewProjection, const unsigned int currentShaderID, float size)
{
	UpdateBuffers();

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
	return fp;
}

FastPoint* PointSystem::GetPointOnce()
{
	FastPoint* fp = &pointsContainer[FindUnused()];
	fp->DrawOnce();
	return fp;
}

void PointSystem::Update()
{
	ActiveCount = UpdateContainer();
}
