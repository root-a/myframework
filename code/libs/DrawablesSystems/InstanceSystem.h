#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "Node.h"
#include "MinMax.h"
#include "Vao.h"
#include "Material.h"

class OBJ;
class LocationLayout;
class VertexBufferDynamic;

class InstanceSystem : public Component
{
public:
	InstanceSystem();
	InstanceSystem(int maxCount, OBJ* object);
	~InstanceSystem();
	void SetUp(int maxCount, OBJ* object);
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
	Component* Clone();
	int LastUsed;
	unsigned int MaxCount;

	VertexArray vao;
	Material mat;

	Object* objectContainer;

	VertexBufferDynamic* modelBuffer;
	VertexBufferDynamic* objectIDBuffer;
	VertexBufferDynamic* materialColorBuffer;
	LocationLayout* model;
	LocationLayout* id;
	LocationLayout* color;
	bool dirty = false;
	bool paused = true;
};