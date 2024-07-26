#pragma once
#include "MyMathLib.h"
#include "LineNode.h"
#include "Component.h"
#include "Vao.h"

class Material;
class LocationLayout;
class VertexBufferDynamic;

class FastPoint
{
public:
	FastPoint()
	{
		data.color = glm::vec4(0.f, 3.f, 3.f, 0.1f);
		draw = false;
		drawAlways = false;
	};
	~FastPoint(){};
	struct PointData
	{
		glm::vec3 position;
		glm::vec4 color;
	};

	PointData data;

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
	void Draw(const glm::mat4& ViewProjection, const unsigned int currentShaderID, float size = 10.0f);
	FastPoint* GetPoint();
	FastPoint* GetPointOnce();
	void UpdateContainer();
	void Update();
	Component* Clone();

	int LastUsed;
	unsigned int MaxCount;
	FastPoint* pointsContainer;
	
	VertexArray vao;
	VertexBufferDynamic* positionColorBuffer;

	unsigned int ViewProjectionHandle;

	bool dirty = false;
	bool paused = true;
};
