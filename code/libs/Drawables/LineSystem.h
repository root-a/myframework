#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "LineNode.h"

class Mesh;
class Material;
class Node;

class FastLine
{
public:
	FastLine()
	{ 
		colorA = mwm::Vector4(0.f, 3.f, 3.f, 0.1f);
		colorB = mwm::Vector4(3.f, 3.f, 0.f, 0.1f);
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
	mwm::Vector3 GetPositionA();
	mwm::Vector3 GetPositionB();

	mwm::Vector4 colorA;
	mwm::Vector4 colorB;
	LineNode localNodeA;
	LineNode localNodeB;

	void StopDrawing() { draw = false; drawAlways = false; }
	void DrawOnce() { draw = true; drawAlways = false; }
	void DrawAlways() { draw = true; drawAlways = true; }

	bool CanDraw() { return draw; }
	bool CanDrawAlways() { return drawAlways; }
	void UpdateDrawState() { draw = drawAlways; }
	
	float cameraDistance;

	bool operator<(FastLine& that){
		// Sort in reverse order : far particles drawn first.
		return this->cameraDistance > that.cameraDistance;
	}
	void* operator new(size_t i){ return _mm_malloc(i, 16); }
	void operator delete(void* p) { _mm_free(p); }
protected:
private:
	bool draw;
	bool drawAlways;
	Node* nodeA;
	Node* nodeB;
};


class LineSystem : public Component
{
	
public:
	
	LineSystem(int maxCount);
	~LineSystem();
	int FindUnused();
	void SetUpBuffers();
	void UpdateBuffers();
	void Draw(const mwm::Matrix4& ViewProjection, const unsigned int currentShaderID, float width = 4.f);
	FastLine* GetLine();
	FastLine* GetLineOnce();
	int UpdateContainer();
	void Update();

	static const mwm::Vector3 vertices[2];

	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;

	int LastUsed;
	int ActiveCount;
	int MaxCount;
	mwm::Vector3* positions;
	mwm::Vector4* colors;
	FastLine* linesContainer;
	

	unsigned int vaoHandle;
	unsigned int vertexBuffer;
	unsigned int colorBuffer;

	unsigned int ViewProjectionHandle;
};


