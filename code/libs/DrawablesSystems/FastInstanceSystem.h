#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "Node.h"
#include "MinMax.h"
#include <unordered_map>
#include "Vao.h"
#include "Material.h"

class OBJ;
class LocationLayout;
class VertexBufferDynamic;

class FastInstanceSystem : public Component
{
public:
	FastInstanceSystem();
	FastInstanceSystem(int maxCount, OBJ* object);
	~FastInstanceSystem();
	void SetUp(int maxCount, OBJ* object);
	void SetUpGPUBuffers();
	int Draw();
	Object* GetObject();
	void ReturnObject(Object* object);
	void UpdateObject(Object* object);
	void UpdateObjects();
	void ReturnObjects();
	void Update();
	void Init(Object* parent);
	Component* Clone();
	int ActiveCount;
	unsigned int MaxCount;

	VertexArray vao;
	Material mat;

	Object* objectContainer;
	std::unordered_map<Object*, int> indexMap;
	std::vector<Object*> gpuOrderedObjects;
	std::vector<Object*> objectsToReturn;
	std::vector<Object*> objectsToUpdate;

	VertexBufferDynamic* modelBuffer;
	VertexBufferDynamic* objectIDBuffer;
	VertexBufferDynamic* materialColorBuffer;
	LocationLayout* model;
	LocationLayout* id;
	LocationLayout* color;
	bool dirty = false;
	bool paused = true;
};