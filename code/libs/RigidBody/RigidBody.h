#pragma once
#include "MyMathLib.h"
#include "Component.h"

class Material;
class Object;

class RigidBody : public Component
{
public:
	RigidBody();
	~RigidBody();
	
	void SetAwake(const bool awake = true);
	void SetCanSleep(const bool canSleep);
	void ApplyImpulse(const mwm::Vector3& force, const mwm::Vector3& target);
	void ApplyImpulse(const mwm::Vector3& direction, double magnitude, const mwm::Vector3& target);
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
	mwm::Matrix3 inertia_tensor;
	mwm::Vector3 velocity;
	mwm::Vector3 angular_velocity;

	mwm::Vector3 acceleration;
	mwm::Vector3 angular_acc;
	
	bool isAwake;
	mwm::Matrix3 inverse_inertia_tensor_world;
	
	double restitution;
	void SetIsKinematic(bool kinematic);
	bool GetIsKinematic();
private:
	bool isKinematic;
	double motion;
	bool canSleep;
	double sleepEpsilon;
	void UpdateKineticEnergyStoreAndPutToSleep(double timestep);
	void UpdateInertiaTensor();
	void SetInertiaTensor(const mwm::Matrix3& I);
	void IntegrateEuler(double timestep, const mwm::Vector3& gravity);
	void IntegrateMid(double timestep, const mwm::Vector3& gravity);
	void IntegrateRunge(double timestep, const mwm::Vector3& gravity);
};

