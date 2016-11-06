#pragma once
#include <GL/glew.h>
#include <vector>
#include "MyMathLib.h"
#include "BoundingBox.h"
#include "Node.h"
#include "OBBAABB.h"
class Material;
class Mesh;

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
	mwm::Vector3 boundingSphereOffset;
	void AssignMaterial(Material* mat);
	void AssignMesh(Mesh* mesh);
	int indicesSize;
	
	void setRadius(float radius);	
	
	void IntegrateEuler(float timestep, const mwm::Vector3& gravity);
	void IntegrateMid(float timestep, const mwm::Vector3& gravity);
	void IntegrateRunge(float timestep, const mwm::Vector3& gravity); //acceleration pos and rot
	void setAwake(const bool awake = true);
	void setCanSleep(const bool canSleep);
	void ApplyImpulse(const mwm::Vector3& force, const mwm::Vector3& picking_point);
	void ApplyImpulse(const mwm::Vector3& direction, float magnitude, const mwm::Vector3& point);
	float GetMass();
	float GetMassInverse();
	void SetMass(float mass);
	mwm::Vector3 GetMeshDimensions();
	void SetInertiaTensor(const mwm::Matrix3& I);

	void SetPosition(const mwm::Vector3& vector);
	void SetScale(const mwm::Vector3& vector);
	void Translate(const mwm::Vector3& vector);
	void SetOrientation(const mwm::Quaternion& q);

	mwm::Quaternion GetOrientation();
	mwm::Vector3 extractScale();
	mwm::Vector3 getScale();
	mwm::Vector3 GetPosition() const;

	float mass = 1.f;
	float massInverse = 1.f/1.f;
	float damping = 0.85f;
	mwm::Vector3 accum_force;
	mwm::Vector3 accum_torque;
	mwm::Matrix3 inverse_inertia_tensor;
	mwm::Vector3 velocity;
	mwm::Vector3 angular_velocity;
	
	mwm::Vector3 acceleration;
	mwm::Vector3 angular_acc;
	void SetMeshOffset(const mwm::Vector3& offset);
	mwm::Vector3 ConvertPointToWorld(const mwm::Vector3& point, const mwm::Matrix4& modelTransform);

	AABB aabb;
	OBB obb;
	void UpdateKineticEnergyStoreAndPutToSleep(float timestep);
	void UpdateBoundingBoxes(const BoundingBox& boundingBox);
	void SetOBBHalfExtent(const mwm::Vector3& scale);
	void UpdateInertiaTensor();
	mwm::Matrix4 CalculateOffetedModel() const;
	Object* boundingBoxShape;
	bool isAwake = true;
	mwm::Matrix3 inverse_inertia_tensor_world;
	bool isKinematic = false;
	float restitution = 0.f;

	void UpdatePosAndOrient(float timeStep);
	mwm::Vector3 meshOffset;
	mwm::Matrix4 depthMVP = mwm::Matrix4::identityMatrix();
private:
	float motion = 1.f; //make sure it does not sleep directly at start of simulation
	bool canSleep = true;
	float sleepEpsilon = 0.2f; 
	
	
};

