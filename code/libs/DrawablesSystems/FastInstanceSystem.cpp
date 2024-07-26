#include "FastInstanceSystem.h"
#include <algorithm>
#include <GL/glew.h>
#include "Object.h"
#include "Frustum.h"
#include "CameraManager.h"
#include "GraphicsStorage.h"
#include "GraphicsManager.h"
#include "SceneGraph.h"
#include "OBJ.h"



FastInstanceSystem::FastInstanceSystem()
{
	ActiveCount = 0;
	paused = false;
	objectContainer = nullptr;
}

FastInstanceSystem::FastInstanceSystem(int maxCount, OBJ* object)
{
	MaxCount = maxCount;
	ActiveCount = 0;
	objectContainer = new Object[maxCount];
	paused = false;
	indexMap.reserve(maxCount);
	gpuOrderedObjects.reserve(maxCount);
	objectsToUpdate.reserve(maxCount);
	objectsToReturn.reserve(maxCount);
	GraphicsManager::LoadOBJToVAO(object, &vao);
	SetUpGPUBuffers();
	///mat.AssignTexture(GraphicsStorage::textures.at(0));
}

FastInstanceSystem::~FastInstanceSystem()
{
	delete[] objectContainer;
}

void FastInstanceSystem::SetUp(int maxCount, OBJ* object)
{
	MaxCount = maxCount;
	ActiveCount = 0;
	objectContainer = new Object[maxCount];
	paused = false;
	indexMap.reserve(maxCount);
	gpuOrderedObjects.reserve(maxCount);
	objectsToUpdate.reserve(maxCount);
	objectsToReturn.reserve(maxCount);
	GraphicsManager::LoadOBJToVAO(object, &vao);
	SetUpGPUBuffers();
	///mat.AssignTexture(GraphicsStorage::textures.at(0));
}

//this is initial
void FastInstanceSystem::SetUpGPUBuffers()
{
	BufferLayout vbModel({ {ShaderDataType::Type::FloatMat4, "M", 1} });
	BufferLayout vbID({ {ShaderDataType::Type::Int, "ID", 1} });
	BufferLayout vbColor({ {ShaderDataType::Type::Float4, "MaterialColor", 1} });
	
	modelBuffer = GraphicsStorage::assetRegistry.AllocAsset<VertexBufferDynamic>(nullptr, MaxCount, vbModel);
	objectIDBuffer = GraphicsStorage::assetRegistry.AllocAsset<VertexBufferDynamic>(nullptr, MaxCount, vbID);
	materialColorBuffer = GraphicsStorage::assetRegistry.AllocAsset<VertexBufferDynamic>(nullptr, MaxCount, vbColor);
	
	vao.AddVertexBuffer(modelBuffer);
	vao.AddVertexBuffer(objectIDBuffer);
	vao.AddVertexBuffer(materialColorBuffer);
	
	model = &modelBuffer->layout.locations[0];
	id = &objectIDBuffer->layout.locations[1];
	color = &materialColorBuffer->layout.locations[0];
}

int FastInstanceSystem::Draw()
{
	if (dirty)
	{
		//UpdateCPUBuffers();
		//UpdateGPUBuffers();
		UpdateObjects();
		ReturnObjects();
		dirty = false;
	}
	
	vao.Bind();
	vao.activeCount = ActiveCount;
	vao.Draw();
	
	return ActiveCount;
}

//this is for runtime
Object* FastInstanceSystem::GetObject()
{
	if (ActiveCount < MaxCount)
	{
		Object* object = &objectContainer[ActiveCount];
		indexMap[object] = ActiveCount;
		objectsToUpdate.push_back(object);
		gpuOrderedObjects.push_back(object);
		dirty = true;
		modelBuffer->IncreaseInstanceCount();
		objectIDBuffer->IncreaseInstanceCount();
		return object;
	}
	else
	{
		Object* object = &objectContainer[MaxCount - 1];
		objectsToUpdate.push_back(object);
		dirty = true;
		return object;
	}
}

//this is for runtime
void FastInstanceSystem::ReturnObject(Object* object)
{
	objectsToReturn.push_back(object);
	dirty = true;
}

//this is for runtime
void FastInstanceSystem::UpdateObject(Object* object)
{
	objectsToUpdate.push_back(object);
	dirty = true;
}

//this is for runtime
void FastInstanceSystem::UpdateObjects()
{
	for (auto object : objectsToUpdate)
	{
		std::unordered_map<Object*, int>::iterator it = indexMap.find(object);
		if (it == indexMap.end()) continue;

		int index = it->second;

		modelBuffer->SetData(index, *model, &object->node->TopDownTransformF);
		objectIDBuffer->SetData(index, *id, &object->ID);
		//glNamedBufferSubData(materialColorBuffer, index * sizeof(Vector4F), sizeof(Vector4F), &object->mat->colorShininess);
	}
	objectsToUpdate.clear();
}

//this is for runtime
void FastInstanceSystem::ReturnObjects()
{
	for (auto object : objectsToReturn)
	{
		std::unordered_map<Object*, int>::iterator it = indexMap.find(object);

		if (it == indexMap.end()) continue;

		int index = it->second;

		ActiveCount--;

		//if (index == ActiveCount) continue;

		//Scene::Instance()->unregisterForPicking(object);
		indexMap[object] = ActiveCount;
		Object* lastObject = gpuOrderedObjects[ActiveCount];
		indexMap[lastObject] = index; //we have no good way of knowing the last object, i know the order via map

		modelBuffer->SetData(index, *model, &lastObject->node->TopDownTransformF);
		objectIDBuffer->SetData(index, *id, &lastObject->ID);
		modelBuffer->activeCount--;
		objectIDBuffer->activeCount--;
		//glNamedBufferSubData(materialColorBuffer, index * sizeof(Vector4F), sizeof(Vector4F), &lastObject->mat->colorShininess);
	
		gpuOrderedObjects[ActiveCount] = object;
		gpuOrderedObjects[index] = lastObject;
	}
	objectsToReturn.clear();
}

void FastInstanceSystem::Update()
{
	/*
	if (!paused)
	{
		UpdateCPUBuffers();
		UpdateGPUBuffers();
	}
	*/
}

void FastInstanceSystem::Init(Object * parent)
{
	Component::Init(parent);

	for (size_t i = 0; i < MaxCount; i++)
	{
		Material* newMaterial = GraphicsStorage::assetRegistry.AllocAsset<Material>();
		newMaterial->name = "fastInsSysMat";
		*newMaterial = mat;
		objectContainer[i].AssignMaterial(newMaterial);
		objectContainer[i].node->SetScale(SceneGraph::Instance()->generateRandomIntervallVectorSpherical(1, 15));
		objectContainer[i].node->SetPosition(SceneGraph::Instance()->generateRandomIntervallVectorSpherical(2, 15));
		objectContainer[i].bounds->SetUp(parent->bounds->centerOfMesh, parent->bounds->dimensions, parent->bounds->name);
		objectContainer[i].node->UpdateNode(*parent->node);
		SceneGraph::Instance()->registerForPicking(&objectContainer[i]);
	}
	for (size_t i = 0; i < MaxCount/10; i++)
	{
		GetObject();
	}
}

Component* FastInstanceSystem::Clone()
{
	return new FastInstanceSystem(*this);
}
