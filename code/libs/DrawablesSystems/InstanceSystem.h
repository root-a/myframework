#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "Node.h"
#include "MinMax.h"
#include "Vao.h"
#include "Material.h"

class OBJ;

class InstanceSystem : public Component
{
public:
	InstanceSystem(int maxCount, OBJ* object);
	~InstanceSystem();
	int FindUnused();
	void SetUpGPUBuffers();
	void UpdateGPUBuffers();
	int Draw();
	Object* GetObject();
	Object* GetObjectOnce();
	void UpdateCPUBuffers();
	void UpdateCPUBuffersNoCulling();
	void Update();
	void Init(Object* parent);
	int LastUsed;
	int ActiveCount;
	int MaxCount;

	Vao vao;
	Material mat;

	Matrix4F* M;
	unsigned int* objectID;
	Vector4F* materialColorShininess;

	Object* objectContainer;

	unsigned int modelBuffer;
	unsigned int objectIDBuffer;
	unsigned int materialColorBuffer;

	bool dirty = false;
	bool paused = true;
};