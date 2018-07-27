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
		colorA = mwm::Vector4F(0.f, 3.f, 3.f, 0.1f);
		colorB = mwm::Vector4F(3.f, 3.f, 0.f, 0.1f);
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
	void SetPositionA(mwm::Vector3& pos);
	void SetPositionB(mwm::Vector3& pos);

	mwm::Vector4F colorA;
	mwm::Vector4F colorB;
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
	
	float cameraDistance;

	bool operator<(FastLine& that){
		// Sort in reverse order : far particles drawn first.
		return this->cameraDistance > that.cameraDistance;
	}
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
	void Draw(const mwm::Matrix4& ViewProjection, const unsigned int currentShaderID, float width = 4.f);
	FastLine* GetLine();
	FastLine* GetLineOnce();
	int UpdateContainer();
	void Update();

	static const mwm::Vector3F vertices[2];

	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;

	int LastUsed;
	int ActiveCount;
	int MaxCount;
	mwm::Vector3F* positions;
	mwm::Vector4F* colors;
	FastLine* linesContainer;
	

	unsigned int vaoHandle;
	unsigned int vertexBuffer;
	unsigned int colorBuffer;

	unsigned int ViewProjectionHandle;
};


