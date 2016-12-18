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
	float radius;
	void Update();
	void AddComponent(Component* newComponent);
	void ClearComponents();
	void AssignMaterial(Material* mat);
	void AssignMesh(Mesh* mesh);
	
	void setRadius(float radius);	
	mwm::Vector3 GetMeshDimensions();

	void SetPosition(const mwm::Vector3& vector);
	void SetScale(const mwm::Vector3& vector);

	void CalculateRadius();

	void Translate(const mwm::Vector3& vector);
	void SetOrientation(const mwm::Quaternion& q);

	mwm::Quaternion GetOrientation();
	mwm::Vector3 extractScale();
	mwm::Vector3 getScale();
	mwm::Vector3 GetWorldPosition() const;
	mwm::Vector3 GetLocalPosition() const;
	
	void SetMeshOffset(const mwm::Vector3& offset);
	
	mwm::Matrix4 CalculateOffsettedModel() const;

	mwm::Vector3 meshOffset;
	mwm::Matrix4 depthMVP = mwm::Matrix4::identityMatrix();
	template <typename T>
	T* GetComponent()
	{ 
		for (auto& component : components)
		{
			if (dynamic_cast<T*>(component))
			{
				return (T*)component;
			}
		}
		return NULL;
	}
private:
	std::vector<Component*> components;
	
};

