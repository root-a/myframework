#pragma once
#include "MyMathLib.h"
#include "Node.h"
#include "Component.h"

class Mesh;
class Material;

class FastBoundingBox
{
public:
	FastBoundingBox()
	{
		color = mwm::Vector4(0.f, 3.f, 3.f, 0.1f);
		draw = false;
		drawAlways = false;
	};
	~FastBoundingBox(){};
	Node node;
	mwm::Vector4 color;

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
	void* operator new(size_t i){ return _mm_malloc(i, 16); }
	void operator delete(void* p) { _mm_free(p); }
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
	static const mwm::Vector3 vertices[8];

	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;

	int LastUsed;
	int ActiveCount;
	int MaxCount;

	mwm::Matrix4F* models;
	mwm::Vector4* colors;
	FastBoundingBox* boundingBoxesContainer;

	unsigned int vaoHandle;
	unsigned int vertexBuffer;
	unsigned int modelBuffer;
	unsigned int colorBuffer;
	unsigned int elementBuffer;

	unsigned int ViewProjectionHandle;

	mwm::MinMax CalcValuesInWorld(const mwm::Matrix3& modelMatrix, const mwm::Vector3& position) const;	
};
