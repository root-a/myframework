#include "Object.h"
#include "Node.h"
#include "Material.h"
#include "Mesh.h"
#include <cmath> 
#include "OBJ.h"
#include "RigidBody.h"
#include <algorithm>
#include <math.h>

using namespace mwm;

Object::Object()
{
	//radius = 1.f;
	mesh = nullptr;
}

Object::~Object()
{
}

void Object::AssignMaterial(Material* mat)
{
	this->mat = mat;
}

void Object::AssignMesh(Mesh* mesh)
{
	this->mesh = mesh;
	this->node.meshCenter = mesh->obj->center_of_mesh;
	CalculateRadius();
}

Vector3 Object::extractScale()
{
	return node.TopDownTransform.extractScale();
}

Vector3 Object::getScale()
{
	return this->node.totalScale;
}

mwm::Vector3 Object::GetLocalScale()
{
	return this->node.localScale;
}

void Object::SetPosition(const Vector3& vector )
{
	this->node.position = vector;
}

Vector3 Object::GetWorldPosition() const
{
	return this->node.TopDownTransform.getPosition();
}


mwm::Vector3 Object::GetLocalPosition() const
{
	return this->node.position;
}

void Object::SetScale(const Vector3& vector )
{
	Vector3 parentScale = this->node.totalScale / this->node.localScale;
	this->node.localScale = vector;
	this->node.totalScale = this->node.localScale * parentScale;

	CalculateRadius();
}

void Object::Translate(const Vector3& vector)
{
	this->node.position += vector;
}

Vector3 Object::GetMeshDimensions()
{
	return this->mesh->obj->GetDimensions();
}

void Object::SetOrientation(const Quaternion& q)
{
	this->node.orientation = q;
}

Quaternion Object::GetLocalOrientation()
{
	return this->node.orientation;
}

mwm::Matrix3 Object::GetWorldRotation3()
{
	return this->node.TopDownTransform.extractRotation3();
}

mwm::Matrix4 Object::GetWorldRotation()
{
	return this->node.TopDownTransform.extractRotation();
}

mwm::Quaternion Object::GetWorldOrientation()
{
	return this->node.TopDownTransform.extractRotation3().toQuaternion();
}

void Object::AddComponent(Component* newComponent)
{
	node.components.push_back(newComponent);
	newComponent->object = this;
}

void Object::Update()
{
	CalculateRadius();
}

void Object::CalculateRadius()
{
	if (mesh != nullptr)
	{
		Vector3 halfExtents = (mesh->obj->dimensions*node.totalScale)*0.5;
		if (mesh->obj->name.compare("sphere") == 0)
		{
			radius = std::max(std::max(halfExtents.x, halfExtents.y), halfExtents.z); //perfect for sphere //picking max component in case sphere was not scaled uniformly
		}
		else
		{
			radius = halfExtents.vectLengt(); //perfect for cuboid
		}
	}
}
