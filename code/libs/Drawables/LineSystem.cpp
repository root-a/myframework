#include "LineSystem.h"
#include "Mesh.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>
#include "Node.h"

using namespace mwm;
LineSystem::LineSystem(int maxCount){

	MaxCount = maxCount;
	LastUsed = 0;
	ActiveCount = 0;
	linesContainer = new FastLine[maxCount];
	positions = new Vector3F[maxCount * 2];
	colors = new Vector4F[maxCount * 2];
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

const Vector3F LineSystem::vertices[] = {
	Vector3F(0.f, 0.f, 0.f),
	Vector3F(0.f, 0.f, 1.f)
};

int LineSystem::FindUnused()
{
	for (int i = LastUsed; i < MaxCount; i++){
		if (linesContainer[i].CanDraw() == false){
			LastUsed = i;
			return i;
		}
	}

	for (int i = 0; i < LastUsed; i++){
		if (linesContainer[i].CanDraw() == false){
			LastUsed = i;
			return i;
		}
	}
	return 0;
}

int LineSystem::UpdateContainer()
{
	int LinesCount = 0;
	for (int i = 0; i < MaxCount; i++){

		FastLine& l = linesContainer[i];

		if (l.CanDraw())
		{
			positions[LinesCount] = l.GetPositionA().toFloat();
			positions[LinesCount + 1] = l.GetPositionB().toFloat();

			colors[LinesCount] = l.colorA;
			colors[LinesCount + 1] = l.colorB;

			LinesCount += 2;
			l.UpdateDrawState();
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
	glBufferData(GL_ARRAY_BUFFER, MaxCount * 2 * sizeof(Vector3F), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, MaxCount * 2 * sizeof(Vector4F), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void LineSystem::UpdateBuffers()
{
	glBindVertexArray(vaoHandle);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Vector3F), positions);

	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Vector4F), colors);
}

void LineSystem::Draw(const mwm::Matrix4 & ViewProjection, const unsigned int currentShaderID, float width)
{
	glBindVertexArray(vaoHandle);

	ViewProjectionHandle = glGetUniformLocation(currentShaderID, "VP");
	glUniformMatrix4fv(ViewProjectionHandle, 1, GL_FALSE, &ViewProjection.toFloat()[0][0]);

	glLineWidth(width);
	glDrawArrays(GL_LINES, 0, ActiveCount);
	glLineWidth(1.f);
}

FastLine* LineSystem::GetLine()
{
	FastLine* fl = &linesContainer[FindUnused()];
	fl->DrawAlways();
	return fl;
}

void LineSystem::Update()
{
	ActiveCount = UpdateContainer();

	UpdateBuffers();
}

FastLine* LineSystem::GetLineOnce()
{
	FastLine* fl = &linesContainer[FindUnused()];
	fl->DrawOnce();
	return fl;
}

void FastLine::AttachEndA(Node* node)
{
	nodeA = node;
}

void FastLine::AttachEndB(Node* node)
{
	nodeB = node;
}

void FastLine::DetachEndA()
{
	localNodeA.TopDownTransform = nodeA->TopDownTransform;
	nodeA = &localNodeA;
}

void FastLine::DetachEndB()
{
	localNodeB.TopDownTransform = nodeB->TopDownTransform;
	nodeB = &localNodeB;
}

mwm::Vector3 FastLine::GetPositionA()
{
	return nodeA->TopDownTransform.getPosition() + localNodeA.position;
}

mwm::Vector3 FastLine::GetPositionB()
{
	return nodeB->TopDownTransform.getPosition() + localNodeB.position;
}

void FastLine::SetPositionA(mwm::Vector3& pos)
{
	localNodeA.position = pos;
}

void FastLine::SetPositionB(mwm::Vector3 & pos)
{
	localNodeB.position = pos;
}

//we can attach lines to objects
//if line uses object node we can then attach other lines to that line.
//using localNodes we can apply some offsets
//lines can be detached and they will maintain the offsets as well
//main issue right now is that lines can be attached to the objects and not other lines as there is no way to update them until they are treated as objects
//we could update them in separate update function treating it as second scene graph and therefore would not interact with default scene graph