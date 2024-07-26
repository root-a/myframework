#include "LineSystem.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>
#include "Node.h"
#include "GraphicsStorage.h"

LineSystem::LineSystem(int maxCount){

	MaxCount = maxCount;
	LastUsed = 0;
	linesContainer = new FastLine[maxCount];
	vao.SetPrimitiveMode(PrimitiveMode::LINES);
	SetUpBuffers();
}

LineSystem::~LineSystem()
{
	delete[] linesContainer;
}

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

void LineSystem::UpdateContainer()
{
	positionColorBuffer->activeCount = 0;
	for (int i = 0; i < MaxCount; i++){

		FastLine& l = linesContainer[i];

		if (l.CanDraw())
		{
			l.data.positionA = l.GetPositionA();
			l.data.positionB = l.GetPositionB();
			positionColorBuffer->SetData(positionColorBuffer->activeCount, &l.data, sizeof(FastLine::LineData));
			positionColorBuffer->IncreaseInstanceCount();
			positionColorBuffer->IncreaseInstanceCount();
			l.UpdateDrawState();
		}
	}
}

void LineSystem::SetUpBuffers()
{
	BufferLayout vbPositionColor({ {ShaderDataType::Type::Float3, "Position"}, {ShaderDataType::Type::Float4, "Color"} });
	positionColorBuffer = GraphicsStorage::assetRegistry.AllocAsset<VertexBufferDynamic>(nullptr, MaxCount * 2, vbPositionColor);
	vao.AddVertexBuffer(positionColorBuffer);
}

void LineSystem::UpdateBuffers()
{
	positionColorBuffer->Update();
}

void LineSystem::Draw(const glm::mat4& ViewProjection, const unsigned int currentShaderID)
{
	if (dirty)
	{
		UpdateContainer();
		UpdateBuffers();
		dirty = false;
	}
	vao.Bind();
	vao.activeCount = positionColorBuffer->activeCount;
	//ViewProjectionHandle = glGetUniformLocation(currentShaderID, "VP");
	//glUniformMatrix4fv(ViewProjectionHandle, 1, GL_FALSE, &ViewProjection.toFloat()[0][0]);
	vao.Draw();
}

FastLine* LineSystem::GetLine()
{
	FastLine* fl = &linesContainer[FindUnused()];
	fl->DrawAlways();
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

Component* LineSystem::Clone()
{
	return new LineSystem(*this);
}

FastLine* LineSystem::GetLineOnce()
{
	FastLine* fl = &linesContainer[FindUnused()];
	fl->DrawOnce();
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

glm::vec3 FastLine::GetPositionA()
{
	return MathUtils::GetPosition(nodeA->TopDownTransform) + localNodeA.localPosition;
}

glm::vec3 FastLine::GetPositionB()
{
	return MathUtils::GetPosition(nodeB->TopDownTransform) + localNodeB.localPosition;
}

void FastLine::SetPositionA(const glm::vec3& pos)
{
	localNodeA.localPosition = pos;
}

void FastLine::SetPositionB(const glm::vec3& pos)
{
	localNodeB.localPosition = pos;
}

//we can attach lines to objects
//if line uses object node we can then attach other lines to that line.
//using localNodes we can apply some offsets
//lines can be detached and they will maintain the offsets as well
//main issue right now is that lines can be attached to the objects and not other lines as there is no way to update them until they are treated as objects
//we could update them in separate update function treating it as second scene graph and therefore would not interact with default scene graph