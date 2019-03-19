#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "Node.h"
#include "MinMax.h"
#include <unordered_map>

class Material;
class Vao;

class FastInstanceSystem : public Component
{
public:
	FastInstanceSystem(int maxCount);
	~FastInstanceSystem();

	void SetUpGPUBuffers();
	void UpdateGPUBuffers();
	int Draw();
	Object* GetObject();
	void ReturnObject(Object* object);
	void UpdateObject(Object* object);
	void UpdateObjects();
	void ReturnObjects();
	void UpdateCPUBuffers();
	void RetriveObject();
	void Update();
	void Init(Object* parent);

	int ActiveCount;
	int MaxCount;

	Vao* vao;
	Material* mat;

	mwm::Matrix4F* M;
	unsigned int* objectID;
	mwm::Vector3F* materialColor;
	mwm::Vector4F* materialProperties;

	Object* objectContainer;
	std::unordered_map<Object*, int> indexMap;

	unsigned int modelBuffer;
	unsigned int objectIDBuffer;
	unsigned int materialColorBuffer;
	unsigned int materialPropertiesBuffer;
	int firstGetAfterUpdate = 0;
	int nrOfGetsAtUpdate = 0;
	bool dirty = false;
	bool paused = true;
};