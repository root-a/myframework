#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "OBBAABB.h"

class Material;
class Mesh;
class Object;

class RigidBody : public Component
{
public:
	RigidBody(Object* owner);
	~RigidBody();
	
	void SetAwake(const bool awake = true);
	void SetCanSleep(const bool canSleep);
	void ApplyImpulse(const mwm::Vector3& force, const mwm::Vector3& picking_point);
	void ApplyImpulse(const mwm::Vector3& direction, double magnitude, const mwm::Vector3& point);
	double GetMass();
	double GetMassInverse();
	void SetMass(double mass);
	
	void Update();
	double mass;
	double massInverse;
	double linearDamping;
	double angularDamping;
	mwm::Vector3 accum_force;
	mwm::Vector3 accum_torque;
	mwm::Matrix3 inverse_inertia_tensor;
	mwm::Vector3 velocity;
	mwm::Vector3 angular_velocity;

	mwm::Vector3 acceleration;
	mwm::Vector3 angular_acc;

	mwm::AABB aabb;
	mwm::OBB obb;
	
	void UpdateHExtentsAndMass();
	bool isAwake;
	mwm::Matrix3 inverse_inertia_tensor_world;
	bool isKinematic;
	double restitution;

private:
	double motion; //make sure it does not sleep directly at start of simulation
	bool canSleep;
	double sleepEpsilon;
	void UpdateHalfExtents();
	void UpdateKineticEnergyStoreAndPutToSleep(double timestep);
	void UpdateBoundingBoxes();
	void SetOBBHalfExtent(const mwm::Vector3& scale);
	void UpdateInertiaTensor();
	void SetInertiaTensor(const mwm::Matrix3& I);
	void IntegrateEuler(double timestep, const mwm::Vector3& gravity);
	void IntegrateMid(double timestep, const mwm::Vector3& gravity);
	void IntegrateRunge(double timestep, const mwm::Vector3& gravity); //acceleration pos and rot
};

