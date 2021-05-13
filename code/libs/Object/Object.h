#pragma once
#include "MyMathLib.h"
#include "Node.h"
#include <vector>
#include "Bounds.h"
#include <unordered_map>
#include "DataRegistry.h"

class Material;
class Vao;
class Component;
class ObjectProfile;
class Script;

class Object
{
public:
	Object();
	~Object();
	Node* node;
	Node localNode;
	Vao* vao;
	Bounds* bounds;
	unsigned int ID;
	std::string name;

	void AddComponent(Component* newComponent, bool isDynamic = false);
	void SetComponentDynamicState(Component* component, bool isDynamic);
	void RemoveComponent(Component* componentToRemove);
	void AssignMaterial(Material* mat, int slot = 0);
	void AddMaterial(Material* mat);
	void RemoveMaterial(Material* mat);

	std::string& GetName();
	Object* GetParentObject();

	std::vector<Material*> materials;
	std::vector<Component*> components;
	//std::unordered_map<type_info,Component*> componentsMap;
	std::unordered_map<std::string,Component*> componentsMap;
	std::vector<Component*> dynamicComponents;
	
	Component* GetComponent(const char* componentName);
	//Component* GetComponents(const char* componentName);

	template <typename ComponentClassName>
	ComponentClassName* GetComponent()
	{ 
		for (auto& component : components)
		{
			if (dynamic_cast<ComponentClassName*>(component))
			{
				return (ComponentClassName*)component;
			}
		}
		return nullptr;
	}

	template <typename ComponentClassName>
	std::vector<ComponentClassName*> GetComponents()
	{
		std::vector<ComponentClassName*> foundComponents;
		for (auto& component : components)
		{
			if (dynamic_cast<ComponentClassName*>(component))
			{
				foundComponents.push_back((ComponentClassName*)component);
			}
		}
		return foundComponents;
	}

	std::vector<Component*> GetStaticComponents()
	{
		std::vector<Component*> staticComponents;
		for (size_t i = 0; i < components.size(); i++)
		{
			bool found = false;
			for (size_t j = 0; j < dynamicComponents.size(); j++)
			{
				if (components[i] == dynamicComponents[j])
				{
					found = true;
				}
			}
			if (!found)
			{
				staticComponents.push_back(components[i]);
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
	bool inFrustum = false;

	DataRegistry registry;

	Matrix4F TopDownTransformF;

	void LoadLuaFile(const char * filename);

	Script* script;
private:
	int FindDynamicComponentIndex(Component * componentToFind);
	int FindComponentIndex(Component * componentToFind);
	int FindMaterialIndex(Material* materialToFind);
	static unsigned int currentID;
	bool draw = false;
	bool drawAlways = false;
protected:

};