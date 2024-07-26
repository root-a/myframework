#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "LineNode.h"
#include "Vao.h"

class Material;
class Node;
class LocationLayout;
class VertexBufferDynamic;

class FastLine
{
public:
	FastLine()
	{ 
		data.colorA = glm::vec4(0.f, 3.f, 3.f, 0.1f);
		data.colorB = glm::vec4(3.f, 3.f, 0.f, 0.1f);
		draw = false;
		drawAlways = false;
		nodeA = &localNodeA;
		nodeB = &localNodeB;
	};
	~FastLine(){};
	void AttachEndA(Node* node);
	void AttachEndB(Node* node);
	void DetachEndA();
	void DetachEndB();
	glm::vec3 GetPositionA();
	glm::vec3 GetPositionB();
	void SetPositionA(const glm::vec3& pos);
	void SetPositionB(const glm::vec3& pos);

	struct LineData
	{
		glm::vec3 positionA;
		glm::vec4 colorA;
		glm::vec3 positionB;
		glm::vec4 colorB;
	};
	LineData data;
	LineNode localNodeA;
	LineNode localNodeB;
	Node* nodeA;
	Node* nodeB;

	void StopDrawing() { draw = false; drawAlways = false; }
	void DrawOnce() { draw = true; drawAlways = false; }
	void DrawAlways() { draw = true; drawAlways = true; }

	bool CanDraw() { return draw; }
	bool CanDrawAlways() { return drawAlways; }
	void UpdateDrawState() { draw = drawAlways; }

protected:
private:
	bool draw;
	bool drawAlways;
	
};


class LineSystem : public Component
{
	
public:
	
	LineSystem(int maxCount);
	~LineSystem();
	int FindUnused();
	void SetUpBuffers();
	void UpdateBuffers();
	void Draw(const glm::mat4& ViewProjection, const unsigned int currentShaderID);
	FastLine* GetLine();
	FastLine* GetLineOnce();
	void UpdateContainer();
	void Update();
	Component* Clone();

	int LastUsed;
	unsigned int MaxCount;
	FastLine* linesContainer;
	
	VertexArray vao;
	VertexBufferDynamic* positionColorBuffer;
	unsigned int ViewProjectionHandle;

	bool dirty = false;
	bool paused = true;
};


