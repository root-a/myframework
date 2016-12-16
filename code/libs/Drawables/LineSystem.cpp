#include "LineSystem.h"
#include "Mesh.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>

using namespace mwm;
LineSystem::LineSystem(int maxCount){

	MaxCount = maxCount;
	LastUsed = 0;
	ActiveCount = 0;
	linesContainer = new FastLine[maxCount];
	positions = new Vector3[maxCount * 2];
	colors = new Vector4[maxCount * 2];
	SetUpBuffers();
}

LineSystem::~LineSystem()
{
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vaoHandle);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &colorBuffer);
	delete[] linesContainer;
	delete[] positions;
	delete[] colors;
}

const Vector3 LineSystem::vertices[] = {
	Vector3(0.f, 0.f, 0.f),
	Vector3(0.f, 0.f, 1.f)
};

int LineSystem::FindUnused()
{
	for (int i = LastUsed; i < MaxCount; i++){
		if (linesContainer[i].lifeTime < 0){
			LastUsed = i;
			return i;
		}
	}

	for (int i = 0; i < LastUsed; i++){
		if (linesContainer[i].lifeTime < 0){
			LastUsed = i;
			return i;
		}
	}

	return 0;
}

int LineSystem::UpdateLines()
{
	int LinesCount = 0;
	for (int i = 0; i < MaxCount; i++){

		FastLine& l = linesContainer[i];

		if (l.lifeTime > 0.0f)
		{
			positions[LinesCount] = l.nodeA.position;
			positions[LinesCount + 1] = l.nodeB.position;

			colors[LinesCount] = l.colorA;
			colors[LinesCount + 1] = l.colorB;

			LinesCount += 2;
		}
		else{
			l.cameraDistance = -1.0f;
		}
	}
	return LinesCount;
}

void LineSystem::SetUpBuffers()
{
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, MaxCount * 2 * sizeof(Vector3), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, MaxCount * 2 * sizeof(Vector4), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void LineSystem::UpdateBuffers()
{
	//Bind VAO
	glBindVertexArray(vaoHandle);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	//glBufferData(GL_ARRAY_BUFFER, ActiveCount * sizeof(Vector4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Vector3), positions);

	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	//glBufferData(GL_ARRAY_BUFFER, ActiveCount * sizeof(Vector4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Vector4), colors);
}

void LineSystem::Draw(const Matrix4& ViewProjection, const GLuint currentShaderID, float width)
{
	UpdateBuffers();

	ViewProjectionHandle = glGetUniformLocation(currentShaderID, "VP");
	glUniformMatrix4fv(ViewProjectionHandle, 1, GL_FALSE, &ViewProjection.toFloat()[0][0]);
	
	//bind vao before drawing
	glBindVertexArray(vaoHandle);

	glLineWidth(width);
	// Draw the lines !
	//glDrawArraysInstanced(GL_LINES, 0, 2, ActiveCount);

	glDrawArrays(GL_LINES, 0, ActiveCount);
	glLineWidth(1.f);
	//glUseProgram(prevShader);
}

FastLine* LineSystem::GetLine()
{
	return &linesContainer[FindUnused()];
}

void LineSystem::Update()
{
	ActiveCount = UpdateLines();
}


