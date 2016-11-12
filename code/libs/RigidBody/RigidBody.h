#pragma once
#include "MyMathLib.h"
#include "Component.h"

class Material;
class Mesh;
class BoundingBox;
class Object;

class RigidBody : public Component
{
public:
	RigidBody(Object* parent);
	~RigidBody();
	
	void SetAwake(const bool awake = true);
	void SetCanSleep(const bool canSleep);
	void ApplyImpulse(const mwm::Vector3& force, const mwm::Vector3& picking_point);
	void ApplyImpulse(const mwm::Vector3& direction, float magnitude, const mwm::Vector3& point);
	float GetMass();
	float GetMassInverse();
	void SetMass(float mass);
	
	void Update();
	float mass;
	float massInverse;
	float damping;
	mwm::Vector3 accum_force;
	mwm::Vector3 accum_torque;
	mwm::Matrix3 inverse_inertia_tensor;
	mwm::Vector3 velocity;
	mwm::Vector3 angular_velocity;

	mwm::Vector3 acceleration;
	mwm::Vector3 angular_acc;

	mwm::Matrix3 extentsM;
	AABB aabb;
	OBB obb;
	
	void UpdateHExtentsAndMass();
	bool isAwake;
	mwm::Matrix3 inverse_inertia_tensor_world;
	bool isKinematic;
	float restitution;

private:
	float motion; //make sure it does not sleep directly at start of simulation
	bool canSleep;
	float sleepEpsilon;
	void UpdateHalfExtents();
	void UpdateKineticEnergyStoreAndPutToSleep(float timestep);
	void UpdateBoundingBoxes(const BoundingBox& boundingBox);
	void SetOBBHalfExtent(const mwm::Vector3& scale);
	void UpdateInertiaTensor();
	void SetInertiaTensor(const mwm::Matrix3& I);
	void IntegrateEuler(float timestep, const mwm::Vector3& gravity);
	void IntegrateMid(float timestep, const mwm::Vector3& gravity);
	void IntegrateRunge(float timestep, const mwm::Vector3& gravity); //acceleration pos and rot
};

