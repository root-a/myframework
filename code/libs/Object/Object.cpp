#include "Object.h"
#include "Node.h"
#include "Material.h"
#include <cmath> 
#include <algorithm>
#include <math.h>
#include "Component.h"
#include "Bounds.h"
#include "Script.h"



Object::Object()
{
	node = &localNode;
	localNode.owner = this;
	ID = currentID;
	currentID++;
	bounds = nullptr;
	vao = nullptr;
}

Object::~Object()
{
	for (auto& component : components)
	{
		delete component;
	}
	components.clear();
	dynamicComponents.clear();
}

void Object::AssignMaterial(Material* mat, int slot)
{
	if (materials.size() == 0) materials.push_back(mat);
	else if (materials.size() > slot) materials[slot] = mat;
}

void Object::AddMaterial(Material * mat)
{
	materials.push_back(mat);
}

void Object::RemoveMaterial(Material* mat)
{
	int index = FindMaterialIndex(mat);
	if (index != -1)
	{
		materials[index] = materials.back();
		materials.pop_back();
	}
}

std::string & Object::GetName()
{
	return name;
}

Object * Object::GetParentObject()
{
	return node->parent->owner;
}

Component * Object::GetComponent(const char * componentName)
{
	auto component = componentsMap.find(componentName);
	if (component != componentsMap.end()) return component->second;
	return nullptr;
}

void Object::Update()
{
	TopDownTransformF = node->TopDownTransform.toFloat();
}

void Object::UpdateComponents()
{
	for (auto& component : dynamicComponents)
	{
		component->Update();
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

int Object::FindDynamicComponentIndex(Component * componentToFind)
{
	for (size_t i = 0; i < dynamicComponents.size(); i++)
	{
		if (dynamicComponents[i] == componentToFind)
		{
			return i;
		}
	}
	return -1;
}

int Object::FindComponentIndex(Component * componentToFind)
{
	for (size_t i = 0; i < components.size(); i++)
	{
		if (components[i] == componentToFind)
		{
			return i;
		}
	}
	return -1;
}

int Object::FindMaterialIndex(Material* materialToFind)
{
	for (size_t i = 0; i < materials.size(); i++)
	{
		if (materials[i] == materialToFind)
		{
			return i;
		}
	}
	return -1;
}

void Object::LoadLuaFile(const char * filename)
{
	std::string directorywithfilename = "resources\\objects\\scripts";
	directorywithfilename.append(filename);
	directorywithfilename.append(".lua");

	script->LoadLuaFile(directorywithfilename.c_str());
}

void Object::AddComponent(Component* newComponent, bool isDynamic)
{
	components.push_back(newComponent);
	if (isDynamic)
		dynamicComponents.push_back(newComponent);
	newComponent->Init(this);
}

void Object::SetComponentDynamicState(Component * component, bool isDynamic)
{
	if (isDynamic)
	{
		dynamicComponents.push_back(component);
	}
	else
	{
		for (size_t i = 0; i < dynamicComponents.size(); i++)
		{
			if (component == dynamicComponents[i])
			{
				dynamicComponents[i] = dynamicComponents.back();
				dynamicComponents.pop_back();
				return;
			}
		}
	}
}

void Object::RemoveComponent(Component * componentToRemove)
{
	int index = FindDynamicComponentIndex(componentToRemove);
	if (index != -1)
	{
		dynamicComponents[index] = dynamicComponents.back();
		dynamicComponents.pop_back();
	}
	index = FindComponentIndex(componentToRemove);
	if (index != -1)
	{
		components[index] = components.back();
		components.pop_back();
	}
	for (auto& componentPair : componentsMap)
	{
		if (componentPair.second == componentToRemove)
		{
			componentsMap.erase(componentPair.first);
			break;
		}
	}
}

unsigned int Object::currentID = 0;