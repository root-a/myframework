#include "Object.h"
#include "Node.h"
#include "Material.h"
#include <cmath> 
#include <algorithm>
#include <math.h>
#include "Component.h"
#include "Bounds.h"
using namespace mwm;

Object::Object()
{
	node = &localNode;
	vao = nullptr;
	ID = currentID;
	currentID++;
	bounds = new Bounds();
	AddComponent(bounds);
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

void Object::AssignMaterial(Material* mat)
{
	this->mat = mat;
}

void Object::AssignMesh(Vao* mesh)
{
	vao = mesh;
}

void Object::Attach(Node * nodeToAttachTo)
{
	node = nodeToAttachTo;
}

void Object::Attach(Object * objectToAttachTo)
{
	node = objectToAttachTo->node;
}

void Object::Detach()
{
	node->SetPosition(node->GetWorldPosition());
	node->SetOrientation(node->GetWorldOrientation());
	node->SetScale(node->getScale());
	node = &localNode;
}

void Object::Update()
{
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

void Object::AddComponent(Component* newComponent)
{
	components.push_back(newComponent);
	if (newComponent->IsDynamic())
		dynamicComponents.push_back(newComponent);
	newComponent->Init(this);
}

void Object::UpdateComponentDynamicState(Component* component)
{
	if (component->IsDynamic())
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

unsigned int Object::currentID = 0;