#include "LineSystem.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>
#include "Node.h"


LineSystem::LineSystem(int maxCount){

	MaxCount = maxCount;
	LastUsed = 0;
	ActiveCount = 0;
	linesContainer = new FastLine[maxCount];
	positions = new Vector3F[maxCount * 2];
	colors = new Vector4F[maxCount * 2];
	vao.SetPrimitiveMode(Vao::PrimitiveMode::LINES);
	SetUpBuffers();
}

LineSystem::~LineSystem()
{
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

//we should have a list of indexes to update, maybe?
void LineSystem::UpdateContainer()
{
	ActiveCount = 0;
	for (int i = 0; i < MaxCount; i++){

		FastLine& l = linesContainer[i];

		if (l.CanDraw())
		{
			positions[ActiveCount] = l.GetPositionA().toFloat();
			positions[ActiveCount + 1] = l.GetPositionB().toFloat();

			colors[ActiveCount] = l.colorA;
			colors[ActiveCount + 1] = l.colorB;

			ActiveCount += 2;
			l.UpdateDrawState();
		}
	}
}

void LineSystem::SetUpBuffers()
{
	vertexBuffer = vao.AddVertexBuffer(NULL, MaxCount * 2 * sizeof(Vector3F), { {ShaderDataType::Float3, "Position"} });
	colorBuffer = vao.AddVertexBuffer(NULL, MaxCount * 2 * sizeof(Vector4F), { {ShaderDataType::Float4, "Color"} });
}

void LineSystem::UpdateBuffers()
{
	glNamedBufferSubData(vertexBuffer, 0, ActiveCount * sizeof(Vector3F), positions);
	glNamedBufferSubData(colorBuffer, 0, ActiveCount * sizeof(Vector4F), colors);
}

void LineSystem::Draw(const Matrix4 & ViewProjection, const unsigned int currentShaderID)
{
	if (dirty)
	{
		UpdateContainer();
		UpdateBuffers();
		dirty = false;
	}
	vao.Bind();

	vao.activeCount = ActiveCount;
	vao.Draw();
}

FastLine* LineSystem::GetLine()
{
	FastLine* fl = &linesContainer[FindUnused()];
	fl->DrawAlways();
	fl->nodeA = &fl->localNodeA;
	fl->nodeB = &fl->localNodeB;
	dirty = true;
	return fl;
}

void LineSystem::Update()
{
	if (!paused)
	{
		UpdateContainer();
		UpdateBuffers();
	}
}

FastLine* LineSystem::GetLineOnce()
{
	FastLine* fl = &linesContainer[FindUnused()];
	fl->DrawOnce();
	fl->nodeA = &fl->localNodeA;
	fl->nodeB = &fl->localNodeB;
	dirty = true;
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

Vector3 FastLine::GetPositionA()
{
	return nodeA->TopDownTransform.getPosition() + localNodeA.localPosition;
}

Vector3 FastLine::GetPositionB()
{
	return nodeB->TopDownTransform.getPosition() + localNodeB.localPosition;
}

void FastLine::SetPositionA(Vector3& pos)
{
	localNodeA.localPosition = pos;
}

void FastLine::SetPositionB(Vector3 & pos)
{
	localNodeB.localPosition = pos;
}

//we can attach lines to objects
//if line uses object node we can then attach other lines to that line.
//using localNodes we can apply some offsets
//lines can be detached and they will maintain the offsets as well
//main issue right now is that lines can be attached to the objects and not other lines as there is no way to update them until they are treated as objects
//we could update them in separate update function treating it as second scene graph and therefore would not interact with default scene graph