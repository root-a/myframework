#pragma once
#include "MyMathLib.h"
#include "LineNode.h"
#include "Component.h"

class Mesh;
class Material;

class FastPoint
{
public:
	FastPoint()
	{
		color = mwm::Vector4F(0.f, 3.f, 3.f, 0.1f);
		draw = false;
		drawAlways = false;
	};
	~FastPoint(){};
	LineNode node;
	mwm::Vector4F color;

	void StopDrawing() { draw = false; drawAlways = false; }
	void DrawOnce() { draw = true; drawAlways = false; }
	void DrawAlways() { draw = true; drawAlways = true; }

	bool CanDraw() { return draw; }
	bool CanDrawAlways() { return drawAlways; }
	void UpdateDrawState() { draw = drawAlways; }
	
	float cameraDistance;

	bool operator<(FastPoint& that){
		// Sort in reverse order : far particles drawn first.
		return this->cameraDistance > that.cameraDistance;
	}
protected:
private:
	bool draw;
	bool drawAlways;
	
};

class PointSystem : public Component
{

public:

	PointSystem(int maxCount);
	~PointSystem();
	int FindUnused();
	void SetUpBuffers();
	void UpdateBuffers();
	void Draw(const mwm::Matrix4& ViewProjection, const unsigned int currentShaderID, float size = 10.0f);
	FastPoint* GetPoint();
	FastPoint* GetPointOnce();
	int UpdateContainer();
	void Update();

	static const mwm::Vector3F vertices[1];

	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;

	int LastUsed;
	int ActiveCount;
	int MaxCount;
	mwm::Vector3F* positions;
	mwm::Vector4F* colors;
	FastPoint* pointsContainer;


	unsigned int vaoHandle;
	unsigned int vertexBuffer;
	unsigned int colorBuffer;

	unsigned int ViewProjectionHandle;
};
