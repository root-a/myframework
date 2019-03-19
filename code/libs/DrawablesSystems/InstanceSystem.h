#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "Node.h"
#include "MinMax.h"
class Material;
class Vao;

class InstanceSystem : public Component
{
public:
	InstanceSystem(int maxCount);
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

	Vao* vao;
	Material* mat;

	mwm::Matrix4F* M;
	unsigned int* objectID;
	mwm::Vector3F* materialColor;
	mwm::Vector4F* materialProperties;

	Object* objectContainer;

	unsigned int modelBuffer;
	unsigned int objectIDBuffer;
	unsigned int materialColorBuffer;
	unsigned int materialPropertiesBuffer;

	bool dirty = false;
	bool paused = true;
};