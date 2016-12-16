#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "LineNode.h"

class Mesh;
class Material;

class FastLine
{
public:
	FastLine()
	{ 
		colorA = mwm::Vector4(0.f, 3.f, 3.f, 0.1f);
		colorB = mwm::Vector4(3.f, 3.f, 0.f, 0.1f);
		lifeTime = -1.0f;
	};
	~FastLine(){};
	LineNode nodeA;
	LineNode nodeB;
	mwm::Vector4 colorA;
	mwm::Vector4 colorB;

	float lifeTime;
	float cameraDistance;

	bool operator<(FastLine& that){
		// Sort in reverse order : far particles drawn first.
		return this->cameraDistance > that.cameraDistance;
	}
	void* operator new(size_t i){ return _mm_malloc(i, 16); }
	void operator delete(void* p) { _mm_free(p); }
protected:
private:
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
	int UpdateLines();
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
	unsigned int positionBuffer;
	unsigned int colorBuffer;

	unsigned int ViewProjectionHandle;
};


