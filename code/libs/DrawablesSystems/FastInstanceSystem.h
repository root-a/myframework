#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "Node.h"
#include "MinMax.h"
#include <unordered_map>
#include "Vao.h"
#include "Material.h"

class OBJ;

class FastInstanceSystem : public Component
{
public:
	FastInstanceSystem(int maxCount, OBJ* object);
	~FastInstanceSystem();

	void SetUpGPUBuffers();
	int Draw();
	Object* GetObject();
	void ReturnObject(Object* object);
	void UpdateObject(Object* object);
	void UpdateObjects();
	void ReturnObjects();
	void Update();
	void Init(Object* parent);

	int ActiveCount;
	int MaxCount;

	Vao vao;
	Material mat;

	Object* objectContainer;
	std::unordered_map<Object*, int> indexMap;
	std::vector<Object*> gpuOrderedObjects;
	std::vector<Object*> objectsToReturn;
	std::vector<Object*> objectsToUpdate;

	unsigned int modelBuffer;
	unsigned int objectIDBuffer;
	unsigned int materialColorBuffer;
	bool dirty = false;
	bool paused = true;
};