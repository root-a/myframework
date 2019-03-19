#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "Node.h"
#include "MinMax.h"
#include "Bounds.h"
#include "Vao.h"

class Material;

class FastBoundingBox
{
public:
	FastBoundingBox()
	{
		draw = false;
		drawAlways = false;
		inFrustum = &localInFrustum;
	};
	~FastBoundingBox(){};
	mwm::Matrix4* model;
	mwm::Vector3F* color;
	bool* inFrustum;
	bool localInFrustum = false;

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

class BoundingBoxSystem : public Component
{
	
public:
	
	BoundingBoxSystem(int maxCount);
	~BoundingBoxSystem();
	int FindUnused();
	void SetUpBuffers();
	void UpdateBuffers();
	int Draw(const mwm::Matrix4& ViewProjection, const unsigned int currentShaderID);
	FastBoundingBox* GetBoundingBox();
	FastBoundingBox* GetBoundingBoxOnce();
	void UpdateContainer();
	void Update();

	static const unsigned short elements[24];
	static const mwm::Vector3F vertices[8];

	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;

	int LastUsed;
	int ActiveCount;
	int MaxCount;

	mwm::Matrix4F* models;
	mwm::Vector3F* colors;
	FastBoundingBox* boundingBoxesContainer;

	Vao vao;
	unsigned int vaoHandle;
	unsigned int modelBuffer;
	unsigned int colorBuffer;

	unsigned int ViewProjectionHandle;
	bool dirty = false;
	bool paused = true;
};
