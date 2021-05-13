#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "LineNode.h"
#include "Vao.h"

class Material;
class Node;

class FastLine
{
public:
	FastLine()
	{ 
		colorA = Vector4F(0.f, 3.f, 3.f, 0.1f);
		colorB = Vector4F(3.f, 3.f, 0.f, 0.1f);
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
	Vector3 GetPositionA();
	Vector3 GetPositionB();
	void SetPositionA(Vector3& pos);
	void SetPositionB(Vector3& pos);

	Vector4F colorA;
	Vector4F colorB;
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
	void Draw(const Matrix4& ViewProjection, const unsigned int currentShaderID);
	FastLine* GetLine();
	FastLine* GetLineOnce();
	void UpdateContainer();
	void Update();

	static const Vector3F vertices[2];

	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;

	int LastUsed;
	int ActiveCount;
	int MaxCount;
	Vector3F* positions;
	Vector4F* colors;
	FastLine* linesContainer;
	
	Vao vao;
	unsigned int vertexBuffer;
	unsigned int colorBuffer;

	unsigned int ViewProjectionHandle;

	bool dirty = false;
	bool paused = true;
};


