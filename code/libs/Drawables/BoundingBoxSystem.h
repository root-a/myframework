#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "Node.h"
#include "MinMax.h"
class Mesh;
class Material;

class FastBoundingBox
{
public:
	FastBoundingBox()
	{
		color = mwm::Vector4F(0.f, 3.f, 3.f, 0.1f);
		draw = false;
		drawAlways = false;
	};
	~FastBoundingBox(){};
	Node node;
	mwm::Vector4F color;

	void StopDrawing() { draw = false; drawAlways = false; }
	void DrawOnce() { draw = true; drawAlways = false; }
	void DrawAlways() { draw = true; drawAlways = true; }

	bool CanDraw() { return draw; }
	bool CanDrawAlways() { return drawAlways; }
	void UpdateDrawState() { draw = drawAlways; }
	
	float cameraDistance;

	bool operator<(FastBoundingBox& that){
		// Sort in reverse order : far particles drawn first.
		return this->cameraDistance > that.cameraDistance;
	}
protected:
private:
	bool draw;
	bool drawAlways;
};

class BoundingBoxSystem : public Component
{
	
public:
	
	BoundingBoxSystem(int maxCount);
	~BoundingBoxSystem();
	int FindUnused();
	void SetUpBuffers();
	void UpdateBuffers();
	void Draw(const mwm::Matrix4& ViewProjection, const unsigned int currentShaderID);
	FastBoundingBox* GetBoundingBox();
	FastBoundingBox* GetBoundingBoxOnce();
	int UpdateContainer();
	void Update();

	static const unsigned short elements[24];
	static const mwm::Vector3F vertices[8];

	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;

	int LastUsed;
	int ActiveCount;
	int MaxCount;

	mwm::Matrix4F* models;
	mwm::Vector4F* colors;
	FastBoundingBox* boundingBoxesContainer;

	unsigned int vaoHandle;
	unsigned int vertexBuffer;
	unsigned int modelBuffer;
	unsigned int colorBuffer;
	unsigned int elementBuffer;

	unsigned int ViewProjectionHandle;
};
