#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "Node.h"
#include "MinMax.h"
#include "Bounds.h"
#include "Vao.h"

class Material;
class LocationLayout;
class VertexBufferDynamic;

class FastBoundingBox
{
public:
	FastBoundingBox()
	{
		draw = false;
		drawAlways = false;
	};
	~FastBoundingBox(){};
	
	struct FBBData
	{
		glm::vec3 color;
		glm::mat4 model;
	};

	FBBData data;

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
	int Draw(const glm::mat4& ViewProjection, const unsigned int currentShaderID);
	FastBoundingBox* GetBoundingBox();
	FastBoundingBox* GetBoundingBoxOnce();
	void UpdateContainer();
	void Update();
	Component* Clone();

	int LastUsed;
	unsigned int MaxCount;
	FastBoundingBox* boundingBoxesContainer;

	VertexArray vao;
	VertexBufferDynamic* colorModelBuffer;

	unsigned int ViewProjectionHandle;
	bool dirty = false;
	bool paused = true;
};
