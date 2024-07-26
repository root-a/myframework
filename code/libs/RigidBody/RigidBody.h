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
	void AttractTowardsWithPID(float Kp, float Ki, float Kd, const glm::vec3& target);
	void ApplyImpulse(const glm::vec3& force, const glm::vec3& target);
	void ApplyImpulse(const glm::vec3& direction, float magnitude, const glm::vec3& target);
	double GetMass();
	double GetMassInverse();
	void SetMass(double mass);
	Component* Clone();
	void Update();
	double mass;
	double massInverse;
	double linearDamping;
	double angularDamping;
	glm::vec3 accum_force;
	glm::vec3 accum_torque;
	glm::mat3 inverse_inertia_tensor;
	glm::mat3 inertia_tensor;
	glm::vec3 velocity;
	glm::vec3 angular_velocity;

	glm::vec3 acceleration;
	glm::vec3 angular_acc;

	glm::vec3 integral;
	glm::vec3 prevError;

	bool isAwake;
	glm::mat3 inverse_inertia_tensor_world;
	
	double restitution;
	void SetIsKinematic(bool kinematic);
	bool GetIsKinematic();
private:
	bool isKinematic;
	double motion;
	bool canSleep;
	double sleepEpsilon;
	void UpdateKineticEnergyStoreAndPutToSleep(float timestep);
	void UpdateInertiaTensor();
	void SetInertiaTensor(const glm::mat3& I);
	void IntegrateEuler(float timestep, const glm::vec3& gravity);
	void IntegrateSemiEuler(float timestep, const glm::vec3& gravity);
	void IntegrateMid(float timestep, const glm::vec3& gravity);
	void IntegrateRunge(float timestep, const glm::vec3& gravity);
};

