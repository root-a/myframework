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

//this is initial
void FastInstanceSystem::SetUpGPUBuffers()
{
	modelBuffer = vao.AddVertexBuffer(NULL, MaxCount * sizeof(Matrix4F), { {ShaderDataType::Mat4, "M", 1} });
	objectIDBuffer = vao.AddVertexBuffer(NULL, MaxCount * sizeof(unsigned int), { {ShaderDataType::Int, "ID", 1} });
	materialColorBuffer = vao.AddVertexBuffer(NULL, MaxCount * sizeof(Vector4F), { {ShaderDataType::Float4, "MaterialColor", 1} });
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
		ActiveCount++;
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

		glNamedBufferSubData(modelBuffer, index * sizeof(Matrix4F), sizeof(Matrix4F), &object->node->TopDownTransform.toFloat());
		glNamedBufferSubData(objectIDBuffer, index * sizeof(unsigned int), sizeof(unsigned int), &object->ID);
		glNamedBufferSubData(materialColorBuffer, index * sizeof(Vector4F), sizeof(Vector4F), &object->materials[0]->colorShininess);
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

		//SceneGraph::Instance()->unregisterForPicking(object);
		indexMap[object] = ActiveCount;
		Object* lastObject = gpuOrderedObjects[ActiveCount];
		indexMap[lastObject] = index; //we have no good way of knowing the last object, i know the order via map


		glNamedBufferSubData(modelBuffer, index * sizeof(Matrix4F), sizeof(Matrix4F), &lastObject->node->TopDownTransform.toFloat());
		glNamedBufferSubData(objectIDBuffer, index * sizeof(unsigned int), sizeof(unsigned int), &lastObject->ID);
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
		Material* newMaterial = new Material();
		GraphicsStorage::materials.push_back(newMaterial);
		*newMaterial = mat;
		objectContainer[i].AssignMaterial(newMaterial);
		objectContainer[i].node->SetScale(SceneGraph::Instance()->generateRandomIntervallVectorSpherical(1, 15));
		objectContainer[i].node->SetPosition(SceneGraph::Instance()->generateRandomIntervallVectorSpherical(2, 15));
		objectContainer[i].bounds = new Bounds();
		objectContainer[i].bounds->SetUp(parent->bounds->centerOfMesh, parent->bounds->dimensions, parent->bounds->name);
		objectContainer[i].node->UpdateNode(*parent->node);
		SceneGraph::Instance()->registerForPicking(&objectContainer[i]);
	}
	for (size_t i = 0; i < MaxCount/10; i++)
	{
		GetObject();
	}
}