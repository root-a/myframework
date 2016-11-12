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
}

Object::~Object()
{
	for (auto& component : components)
	{
		delete component;
	}
	components.clear();
}

void Object::AssignMaterial(Material* mat)
{
	this->mat = mat;
}

void Object::AssignMesh(Mesh* mesh)
{
	this->mesh = mesh;
	SetMeshOffset(mesh->obj->CenterOfMass()*-1.f);
	CalculateRadius();
}

void Object::setRadius(float radius)
{
	this->radius = radius;
}

Vector3 Object::extractScale()
{
	return node.TopDownTransform.extractScale();
}

Vector3 Object::getScale()
{
	return this->node.scale;
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
	this->node.scale = vector;
	CalculateRadius();
	if (RigidBody* body = GetComponent<RigidBody>()) body->UpdateHExtentsAndMass();
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

Quaternion Object::GetOrientation()
{
	return this->node.orientation;
}

void Object::SetMeshOffset(const Vector3& offset)
{
	meshOffset = offset;
}

Matrix4 Object::CalculateOffetedModel() const
{
	//apply transformation matrix from node
	//we do physics around the center of the object
	//and in cases when pivot point is not in the center of the object 
	//we have to apply the offset for the graphics to match their physical position 
	Matrix4 offsetMatrix = Matrix4::translate(meshOffset);
	return offsetMatrix*this->node.TopDownTransform;
}

void Object::AddComponent(Component* newComponent)
{
	components.push_back(newComponent);
	newComponent->object = this;
}

void Object::Update()
{
	for (auto& component : components)
	{
		component->Update();
	}
}

void Object::CalculateRadius()
{
	Vector3 halfExtents = (mesh->obj->dimensions*node.scale)*0.5f;
	/*
	float maxRadius = std::max(halfExtents.x, halfExtents.y); 
	radius = std::max(maxRadius, halfExtents.z); //perfect for sphere
	*/
	radius = sqrt(pow(halfExtents.x, 2) + pow(halfExtents.y, 2) + pow(halfExtents.z, 2)); //perfect for cuboid
}
