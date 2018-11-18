#pragma once
#include "MyMathLib.h"
#include "Node.h"
#include <vector>

class Material;
class Mesh;
class Component;

class Object
{
public:
	Object();
	~Object();
	Node node;
	Material* mat;
	Mesh* mesh;
	unsigned int ID;
	
	void Update();
	void AddComponent(Component* newComponent);
	void AssignMaterial(Material* mat);
	void AssignMesh(Mesh* mesh);
	
	mwm::Vector3 GetMeshDimensions();

	void SetPosition(const mwm::Vector3& vector);
	void SetScale(const mwm::Vector3& vector);

	double radius;
	void CalculateRadius();

	void Translate(const mwm::Vector3& vector);
	void SetOrientation(const mwm::Quaternion& q);

	mwm::Matrix3 GetWorldRotation3();
	mwm::Matrix4 GetWorldRotation();
	mwm::Quaternion GetWorldOrientation();
	mwm::Quaternion GetLocalOrientation();
	mwm::Vector3 extractScale();
	mwm::Vector3 getScale();
	mwm::Vector3 GetLocalScale();
	mwm::Vector3 GetWorldPosition() const;
	mwm::Vector3 GetLocalPosition() const;

	template <typename T>
	T* GetComponent()
	{ 
		for (auto& component : node.components)
		{
			if (dynamic_cast<T*>(component))
			{
				return (T*)component;
			}
		}
		return NULL;
	}
private:
	
	
};

