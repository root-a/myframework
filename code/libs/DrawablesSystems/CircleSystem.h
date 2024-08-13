#pragma once
#include "MyMathLib.h"
#include "LineNode.h"
#include "Component.h"
#include "Vao.h"

class Material;
class LocationLayout;
class VertexBufferDynamic;

class FastCircle
{
public:
	FastCircle()
	{
		data.color = glm::vec4(0.f, 3.f, 3.f, 1.0f);
		data.radius = 1.f;
		data.softness = 0.005f;
		data.thickness = 5.0f;
		draw = false;
		drawAlways = false;
		cameraDistance = -1.0;
	};
	~FastCircle(){};

	struct CircleData
	{
		glm::vec3 center;
		float radius;
		glm::vec4 color;
		float softness;
		float thickness;
	};

	CircleData data;

	double cameraDistance;

	void StopDrawing() { draw = false; drawAlways = false; }
	void DrawOnce() { draw = true; drawAlways = false; }
	void DrawAlways() { draw = true; drawAlways = true; }

	bool CanDraw() { return draw; }
	bool CanDrawAlways() { return drawAlways; }
	void UpdateDrawState() { draw = drawAlways; }
	bool operator<(FastCircle& that) {
		// Sort in reverse order : far particles drawn first.
		return this->cameraDistance > that.cameraDistance;
	}
protected:
private:
	bool draw;
	bool drawAlways;
	
};

class CircleSystem : public Component
{

public:

	CircleSystem(int maxCount);
	~CircleSystem();
	int FindUnused();
	void SetUpBuffers();
	void UpdateBuffers();
	void Draw(const glm::mat4& ViewProjection, const unsigned int currentShaderID, float size = 10.0f);
	FastCircle* GetCircle();
	FastCircle* GetCircleOnce();
	void UpdateContainer();
	void Update();
	Component* Clone();

	int LastUsed;
	unsigned int MaxCount;
	FastCircle* circlesContainer;
	
	VertexArray vao;
	unsigned int vertexBuffer;
	VertexBufferDynamic* positionsSizesColorsSoftnessThicknessBuffer;

	bool dirty = false;
	bool paused = true;
};
