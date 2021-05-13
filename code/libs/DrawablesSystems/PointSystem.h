#pragma once
#include "MyMathLib.h"
#include "LineNode.h"
#include "Component.h"
#include "Vao.h"

class Material;

class FastPoint
{
public:
	FastPoint()
	{
		color = Vector4F(0.f, 3.f, 3.f, 0.1f);
		draw = false;
		drawAlways = false;
	};
	~FastPoint(){};
	LineNode node;
	Vector4F color;

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

class PointSystem : public Component
{

public:

	PointSystem(int maxCount);
	~PointSystem();
	int FindUnused();
	void SetUpBuffers();
	void UpdateBuffers();
	void Draw(const Matrix4& ViewProjection, const unsigned int currentShaderID, float size = 10.0f);
	FastPoint* GetPoint();
	FastPoint* GetPointOnce();
	void UpdateContainer();
	void Update();

	static const Vector3F vertices[1];

	unsigned int MatrixHandle;
	unsigned int MaterialColorValueHandle;

	int LastUsed;
	int ActiveCount;
	int MaxCount;
	Vector3F* positions;
	Vector4F* colors;
	FastPoint* pointsContainer;
	
	Vao vao;
	unsigned int vertexBuffer;
	unsigned int colorBuffer;

	unsigned int ViewProjectionHandle;

	bool dirty = false;
	bool paused = true;
};
