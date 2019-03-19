#pragma once
#include "MyMathLib.h"
#include "Node.h"
#include <vector>
#include "Bounds.h"

class Material;
class Vao;
class Component;

class Object
{
public:
	Object();
	~Object();
	Node* node;
	Node localNode;

	Bounds* bounds;
	Material* mat;
	Vao* vao;
	unsigned int ID;

	void AddComponent(Component* newComponent);
	void UpdateComponentDynamicState(Component* component);
	void AssignMaterial(Material* mat);
	void AssignMesh(Vao* mesh);

	void Attach(Node* nodeToAttachTo);
	void Attach(Object* objectToAttachTo);
	void Detach();

	std::vector<Component*> components;
	std::vector<Component*> dynamicComponents;

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

	void Update();
	void UpdateComponents();

	void StopDrawing() { draw = false; drawAlways = false; }
	void DrawOnce() { draw = true; drawAlways = false; }
	void DrawAlways() { draw = true; drawAlways = true; }

	bool CanDraw() { return draw; }
	bool CanDrawAlways() { return drawAlways; }
	void UpdateDrawState() { draw = drawAlways; }
	
	void ResetIDs();
	static unsigned int Count();
	bool inFrustum = false;
	
private:
	static unsigned int currentID;
	bool draw = false;
	bool drawAlways = false;
protected:
	
};