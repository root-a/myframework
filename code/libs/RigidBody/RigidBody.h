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
	void ApplyImpulse(const Vector3& force, const Vector3& target);
	void ApplyImpulse(const Vector3& direction, double magnitude, const Vector3& target);
	double GetMass();
	double GetMassInverse();
	void SetMass(double mass);
	
	void Update();
	double mass;
	double massInverse;
	double linearDamping;
	double angularDamping;
	Vector3 accum_force;
	Vector3 accum_torque;
	Matrix3 inverse_inertia_tensor;
	Matrix3 inertia_tensor;
	Vector3 velocity;
	Vector3 angular_velocity;

	Vector3 acceleration;
	Vector3 angular_acc;
	
	bool isAwake;
	Matrix3 inverse_inertia_tensor_world;
	
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
	void SetInertiaTensor(const Matrix3& I);
	void IntegrateEuler(double timestep, const Vector3& gravity);
	void IntegrateMid(double timestep, const Vector3& gravity);
	void IntegrateRunge(double timestep, const Vector3& gravity);
};

