#pragma once
#include "MyMathLib.h"
#include "Node.h"
#include <vector>
#include "Bounds.h"
#include <unordered_map>
#include <typeindex>
#include "DataRegistry.h"

class Material;
class VertexArray;
class Component;
class ObjectProfile;
class Script;

class Object
{
public:
	Object();
	~Object();
	Node* node;
	//Node localNode;
	VertexArray* vao;
	Bounds* bounds;
	unsigned int ID;
	std::string name;
	std::string path;
	DataRegistry registry;
	template <typename ComponentClassName>
	void AddComponent(ComponentClassName* newComponent, bool isDynamic = false)
	{
		components[typeid(ComponentClassName*)] = newComponent;
		if (isDynamic)
			dynamicComponents[typeid(ComponentClassName*)] = newComponent;
		newComponent->Init(this);
	}
	void AddComponent(const std::type_index& type, Component* newComponent, bool isDynamic = false);
	void SetComponentDynamicState(Component* component, bool isDynamic);
	void SetComponentDynamicState(std::type_index& component, bool isDynamic);
	void RemoveComponent(Component* componentToRemove);
	void RemoveComponent(std::type_index& componentToRemove);
	void AssignMaterial(Material* mat, int sequenceIndex = 0, int materialSlot = 0);
	void UnAssignMaterial(int matSequenceIndex = 0, int slot = 0);
	void AddMaterial(Material* mat, int matSequenceIndex = -1);
	void RemoveMaterial(Material* mat);
	void RemoveMaterialSequence(int index);
	Material* GetMaterial(const char* name);
	Material* GetMaterial(int sequenceIndex, int matIndex = 0);
	std::string& GetName();
	std::string& GetPath();
	Object* GetParentObject();

	std::vector<std::vector<Material*>> materials;
	std::unordered_map<std::type_index, Component*> components;
	std::unordered_map<std::type_index, Component*> dynamicComponents;

	template <typename ComponentClassName>
	ComponentClassName* GetComponent()
	{
		auto res = components.find(typeid(ComponentClassName*));
		return res == components.end() ? nullptr : (ComponentClassName*)res->second;
	}

	std::unordered_map<std::type_index, Component*> GetStaticComponents()
	{
		std::unordered_map<std::type_index, Component*> staticComponents;
		for (auto comp : components)
		{
			bool found = false;
			for (auto dcomp : dynamicComponents)
			{
				if (comp.second == dcomp.second)
				{
					found = true;
				}
			}
			if (!found)
			{
				staticComponents.insert(comp);
			}
		}
		return staticComponents;
	}

	void Update();
	void UpdateComponents();

	void StopDrawing() { draw = false; drawAlways = false; }
	void DrawOnce() { draw = true; drawAlways = false; }
	void DrawAlways() { draw = true; drawAlways = true; }

	bool CanDraw() { return draw; }
	bool CanDrawAlways() { return drawAlways; }
	void UpdateDrawState() { draw = drawAlways; }

	static void ResetIDs();
	static unsigned int Count();
	bool inFrustum = true;

private:
	int FindMaterialIndex(Material* materialToFind, std::vector<Material*>& matSq);
	static unsigned int currentID;
	bool draw = false;
	bool drawAlways = false;
protected:

};