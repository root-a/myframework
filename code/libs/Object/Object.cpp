#include "Object.h"
#include "Node.h"
#include "Material.h"
#include <cmath> 
#include <algorithm>
#include <math.h>
#include "Component.h"
#include "Bounds.h"
#include "ObjectProfile.h"
#include "Script.h"

Object::Object()
{
	ID = currentID;
	currentID++;
	bounds = nullptr;
	vao = nullptr;
}

Object::~Object()
{
}

void Object::AssignMaterial(Material* mat, int sequenceIndex, int materialSlot)
{
	if (materials.size() < sequenceIndex + 1)
	{
		materials.resize(sequenceIndex + 1);
	}
	if (materials[sequenceIndex].size() < materialSlot + 1)
	{
		materials[sequenceIndex].resize(materialSlot + 1);
	}
	materials[sequenceIndex][materialSlot] = mat;
}

void Object::UnAssignMaterial(int matSequenceIndex, int slot)
{
	if (materials.size() > matSequenceIndex)
	{
		if (materials[matSequenceIndex].size() > slot)
		{
			materials[matSequenceIndex].erase(materials[matSequenceIndex].begin() + slot);
			if (materials[matSequenceIndex].size() == 0)
			{
				materials.erase(materials.begin() + matSequenceIndex);
			}
		}
	}
}

void Object::AddMaterial(Material* mat, int matSequenceIndex)
{
	if (matSequenceIndex != -1)
	{
		if (materials.size() < matSequenceIndex + 1)
		{
			materials.resize(matSequenceIndex + 1);
		}
		materials[matSequenceIndex].push_back(mat);
	}
	else
	{
		materials.push_back({ mat });
	}
}

void Object::RemoveMaterial(Material* mat)
{
	std::vector<int> sqToErase;
	int i = 0;
	for (auto& matSq : materials)
	{
		int index = FindMaterialIndex(mat, matSq);
		if (index != -1)
		{
			matSq[index] = matSq.back();
			matSq.pop_back();
			if (matSq.size() == 0)
			{
				sqToErase.push_back(i);
			}
		}
		i++;
	}
	for (auto sqi : sqToErase)
	{
		materials.erase(materials.begin() + sqi);
	}
}

void Object::RemoveMaterialSequence(int index)
{
	materials.erase(materials.begin() + index);
}

Material* Object::GetMaterial(const char* name)
{
	for (auto& matSq : materials)
	{
		for (auto mat : matSq)
		{
			if (strcmp(mat->name.c_str(), name) == 0)
			{
				return mat;
			}
		}
	}
	return nullptr;
}

Material* Object::GetMaterial(int sequenceIndex, int matIndex)
{
	if (materials.size() > sequenceIndex)
	{
		if (materials[sequenceIndex].size() > matIndex)
		{
			return materials[sequenceIndex][matIndex];
		}
	}
	return nullptr;
}

std::string& Object::GetName()
{
	return name;
}

std::string& Object::GetPath()
{
	return path;
}

Object* Object::GetParentObject()
{
	return node->parent->object;
}

void Object::Update()
{
}

void Object::UpdateComponents()
{
	for (auto component : dynamicComponents)
	{
		component.second->Update();
	}
}

void Object::ResetIDs()
{
	currentID = 0;
}

unsigned int Object::Count()
{
	return currentID;
}

int Object::FindMaterialIndex(Material* materialToFind, std::vector<Material*>& matSq)
{
	for (size_t i = 0; i < matSq.size(); i++)
	{
		if (matSq[i] == materialToFind)
		{
			return (int)i;
		}
	}
	return -1;
}

void Object::AddComponent(const std::type_index& type, Component* newComponent, bool isDynamic)
{
	components[type] = newComponent;
	if (isDynamic)
		dynamicComponents[type] = newComponent;
	newComponent->Init(this);
}

void Object::SetComponentDynamicState(Component * component, bool isDynamic)
{
	if (isDynamic)
	{
		for (auto comp : components)
		{
			if (comp.second == component)
			{
				dynamicComponents[comp.first] = component;
				return;
			}
		}
	}
	else
	{
		for (auto comp : dynamicComponents)
		{
			if (component == comp.second)
			{
				dynamicComponents.erase(comp.first);
				return;
			}
		}
	}
}

void Object::SetComponentDynamicState(std::type_index& component, bool isDynamic)
{
	if (isDynamic)
	{
		auto cit = components.find(component);
		if (cit != components.end())
		{
			dynamicComponents[component] = cit->second;
		}
	}
	else
	{
		auto dit = dynamicComponents.find(component);
		if (dit != dynamicComponents.end())
		{
			dynamicComponents.erase(dit);
		}
	}
}

void Object::RemoveComponent(Component * componentToRemove)
{
	for (auto comp : components)
	{
		if (comp.second == componentToRemove)
		{
			components.erase(comp.first);
			auto it = dynamicComponents.find(comp.first);
			if (it != dynamicComponents.end())
			{
				dynamicComponents.erase(it);
			}
			break;
		}
	}
}

void Object::RemoveComponent(std::type_index& componentToRemove)
{
	auto cit = components.find(componentToRemove);
	if (cit != components.end())
	{
		components.erase(cit);
	}
	auto dit = dynamicComponents.find(componentToRemove);
	if (dit != dynamicComponents.end())
	{
		dynamicComponents.erase(dit);
	}
}

unsigned int Object::currentID = 0;