#include "RigidBody.h"
#include <cmath>
#include <algorithm>
#include "Object.h"
#include "PhysicsManager.h"
#include "Times.h"
#include "BoundingBox.h"



RigidBody::RigidBody()
{
	mass = 1;
	massInverse = 1;
	float massSquared = mass * mass;
	inertia_tensor = MathUtils::CuboidInertiaTensor(glm::vec3(1, 1, 1));
	glm::mat3 scaled_inertia_tensor = inertia_tensor * massSquared;
	inverse_inertia_tensor = glm::inverse(scaled_inertia_tensor);
	inverse_inertia_tensor_world = inverse_inertia_tensor;
	angularDamping = 0.85;
	linearDamping = 0.85;
	isAwake = true;
	isKinematic = false;
	restitution = 0.0;
	motion = 1.0; //make sure it does not sleep directly at start of simulation
	canSleep = true;
	sleepEpsilon = 0.2;
	accum_force = glm::vec3(0.0f);
	accum_torque = glm::vec3(0.0f);
	velocity = glm::vec3(0.0f);
	angular_velocity = glm::vec3(0.0f);
	acceleration = glm::vec3(0.0f);
	angular_acc = glm::vec3(0.0f);

	integral = glm::vec3(0.0f);
	prevError = glm::vec3(0.0f);
}

RigidBody::~RigidBody()
{
}

//explicit euler
void RigidBody::IntegrateEuler(float timestep, const glm::vec3& gravity)
{
	//if (!isAwake || isKinematic) return;

	//calculate velocity from forces and acceleration at t1
	this->acceleration = (gravity + this->accum_force) * (float)this->massInverse;
	this->velocity += acceleration * timestep;

	//calculate new position at t1
	this->velocity *= pow(std::clamp(linearDamping, 0.0, 1.0), timestep);
	object->node->Translate(this->velocity * timestep);

	// Calculate the angular acceleration
	this->angular_acc = inverse_inertia_tensor_world*this->accum_torque;

	// Update angular velocity
	this->angular_velocity += angular_acc * timestep;

	// Apply angular damping
	this->angular_velocity *= pow(std::clamp(angularDamping, 0.0, 1.0), timestep);

	// Calculate the angular velocity magnitude and axis
	float angle = glm::length(this->angular_velocity);
	glm::vec3 axis = (angle > 0.0f) ? glm::normalize(this->angular_velocity) : glm::vec3(0.0f);

	// Calculate the quaternion representing the angular rotation
	glm::quat angularRotation = glm::angleAxis(angle * timestep, axis);

	// Normalize the angularRotation quaternion
	angularRotation = glm::normalize(angularRotation);

	// Update the object's orientation using quaternion multiplication
	glm::quat newOrientation = angularRotation * object->node->GetLocalOrientation();
	object->node->SetOrientation(glm::normalize(newOrientation));

	// Clear accumulated forces and torques
	this->accum_force = glm::vec3(0.0, 0.0, 0.0);
	this->accum_torque = glm::vec3(0.0, 0.0, 0.0);

	if (canSleep) UpdateKineticEnergyStoreAndPutToSleep(timestep);
}

//semi-implicit euler
void RigidBody::IntegrateSemiEuler(float timestep, const glm::vec3& gravity)
{
	//if (!isAwake || isKinematic) return;

	// Calculate the angular acceleration
	this->angular_acc = inverse_inertia_tensor_world * this->accum_torque;

	// Update angular velocity
	this->angular_velocity += angular_acc * timestep;

	// Apply angular damping
	this->angular_velocity *= pow(std::clamp(angularDamping, 0.0, 1.0), timestep);

	// Calculate the angular velocity magnitude and axis
	float angle = glm::length(this->angular_velocity);
	glm::vec3 axis = (angle > 0.0f) ? glm::normalize(this->angular_velocity) : glm::vec3(0.0f);

	// Calculate the quaternion representing the angular rotation
	glm::quat angularRotation = glm::angleAxis(angle * timestep, axis);

	// Normalize the angularRotation quaternion
	angularRotation = glm::normalize(angularRotation);

	// Update the object's orientation using quaternion multiplication
	glm::quat newOrientation = angularRotation * object->node->GetLocalOrientation();
	object->node->SetOrientation(glm::normalize(newOrientation));

	// Calculate the linear acceleration
	this->acceleration = (gravity + this->accum_force) * (float)this->massInverse;
	
	// Update linear velocity
	this->velocity += acceleration * timestep;

	// Apply linear damping
	this->velocity *= pow(std::clamp(linearDamping, 0.0, 1.0), timestep);

	// Calculate new position at t1 using semi-implicit Euler
	object->node->Translate(this->velocity * timestep);

	// Clear accumulated forces and torques
	this->accum_force = glm::vec3(0.0, 0.0, 0.0);
	this->accum_torque = glm::vec3(0.0, 0.0, 0.0);

	if (canSleep) UpdateKineticEnergyStoreAndPutToSleep(timestep);
}

void RigidBody::IntegrateMid(float timestep, const glm::vec3& gravity)
{
	//if (!isAwake || isKinematic) return;

	// Calculate the acceleration at the current state
	glm::vec3 acceleration = (gravity + this->accum_force) * (float)this->massInverse;

	// Calculate the velocity at the midpoint using the current acceleration
	glm::vec3 halfStepVelocity = this->velocity + 0.5f * timestep * acceleration;

	// Apply linear damping to the halfStepVelocity
	halfStepVelocity *= pow(std::clamp(linearDamping, 0.0, 1.0), timestep);

	// Calculate the position at the midpoint using the half-step velocity
	glm::vec3 halfStepPosition = this->object->node->GetLocalPosition() + 0.5f * timestep * halfStepVelocity;

	// Calculate the angular acceleration using accumulated torque
	glm::vec3 angularAcceleration = inverse_inertia_tensor_world * this->accum_torque;

	// Calculate the angular velocity at the midpoint using the current angular acceleration
	glm::vec3 halfStepAngularVelocity = this->angular_velocity + 0.5f * timestep * angularAcceleration;

	// Apply angular damping to the halfStepAngularVelocity
	halfStepAngularVelocity *= pow(std::clamp(angularDamping, 0.0, 1.0), timestep);

	// Calculate the orientation at the midpoint using the half-step angular velocity
	glm::quat halfStepOrientation = glm::normalize(this->object->node->GetLocalOrientation() + 0.5f * timestep * glm::quat(0.0f, halfStepAngularVelocity));

	// Calculate the torque at the midpoint using the half-step orientation and accumulated torque
	glm::vec3 halfStepTorque = this->accum_torque + glm::cross(halfStepAngularVelocity, this->inverse_inertia_tensor_world * halfStepAngularVelocity);

	// Calculate the angular acceleration at the midpoint using the half-step orientation and half-step torque
	glm::vec3 halfStepAngularAcceleration = inverse_inertia_tensor_world * halfStepTorque;

	// Update the positions and velocities using the calculated values
	this->velocity += acceleration * timestep;
	this->object->node->SetPosition(halfStepPosition + 0.5f * timestep * this->velocity);

	// Update angular velocity and orientation
	this->angular_velocity += halfStepAngularAcceleration * timestep;
	glm::quat angularOrientation = glm::normalize(halfStepOrientation + 0.5f * timestep * glm::quat(0.0f, this->angular_velocity));

	// Update the object's orientation
	this->object->node->SetOrientation(angularOrientation);

	// Clear accumulated forces and torques
	this->accum_force = glm::vec3(0.0f);
	this->accum_torque = glm::vec3(0.0f);

	if (canSleep) UpdateKineticEnergyStoreAndPutToSleep(timestep);
}

void RigidBody::IntegrateRunge(float timestep, const glm::vec3& gravity)
{
	
	//if (!isAwake || isKinematic) return;
	glm::vec3 acceleration = (gravity + this->accum_force) * (float)this->massInverse;
	glm::vec3 angularAcceleration = inverse_inertia_tensor_world * this->accum_torque;
	glm::vec3 position = this->object->node->GetLocalPosition();
	glm::quat orientation = this->object->node->GetLocalOrientation();

	// Calculate k1 values
	glm::vec3 k1_velocity = this->velocity;
	glm::vec3 k1_angular_velocity = this->angular_velocity;
	glm::vec3 k1_acceleration = acceleration;
	glm::vec3 k1_angular_acceleration = angularAcceleration;

	// Calculate k2 values
    glm::vec3 k2_velocity = this->velocity + 0.5f * timestep * k1_acceleration;
    glm::vec3 k2_angular_velocity = this->angular_velocity + 0.5f * timestep * k1_angular_acceleration;
    glm::vec3 k2_position = position + 0.5f * timestep * k1_velocity;
    glm::quat k2_orientation = glm::normalize(orientation + 0.5f * timestep * glm::quat(0.0f, k1_angular_velocity));
    glm::vec3 k2_acceleration = acceleration;
    glm::vec3 k2_angular_acceleration = angularAcceleration;

	// Calculate k3 values
	glm::vec3 k3_velocity = this->velocity + 0.5f * timestep * k2_acceleration;
	glm::vec3 k3_angular_velocity = this->angular_velocity + 0.5f * timestep * k2_angular_acceleration;
	glm::vec3 k3_position = position + 0.5f * timestep * k2_velocity;
	glm::quat k3_orientation = glm::normalize(orientation + 0.5f * timestep * glm::quat(0.0f, k2_angular_velocity));
	glm::vec3 k3_acceleration = acceleration;
	glm::vec3 k3_angular_acceleration = angularAcceleration;

	// Calculate k4 values
	glm::vec3 k4_velocity = this->velocity + timestep * k3_acceleration;
	glm::vec3 k4_angular_velocity = this->angular_velocity + timestep * k3_angular_acceleration;
	glm::vec3 k4_position = position + timestep * k3_velocity;
	glm::quat k4_orientation = glm::normalize(orientation + timestep * glm::quat(0.0f, k3_angular_velocity));
	glm::vec3 k4_acceleration = acceleration;
	glm::vec3 k4_angular_acceleration = angularAcceleration;

	// Calculate weighted average derivatives for final update
	glm::vec3 avg_velocity = (k1_velocity + 2.0f * k2_velocity + 2.0f * k3_velocity + k4_velocity) / 6.0f;
	glm::vec3 avg_angular_velocity = (k1_angular_velocity + 2.0f * k2_angular_velocity + 2.0f * k3_angular_velocity + k4_angular_velocity) / 6.0f;
	glm::vec3 avg_acceleration = (k1_acceleration + 2.0f * k2_acceleration + 2.0f * k3_acceleration + k4_acceleration) / 6.0f;
	glm::vec3 avg_angular_acceleration = (k1_angular_acceleration + 2.0f * k2_angular_acceleration + 2.0f * k3_angular_acceleration + k4_angular_acceleration) / 6.0f;

	// Apply linear and angular damping
	avg_velocity *= pow(std::clamp(linearDamping, 0.0, 1.0), timestep);
	avg_angular_velocity *= pow(std::clamp(angularDamping, 0.0, 1.0), timestep);

	// Update position and orientation
	glm::vec3 updatedPosition = position + timestep * avg_velocity;
	glm::quat updatedOrientation = glm::normalize(orientation + timestep * glm::quat(0.0f, avg_angular_velocity));

	object->node->SetPosition(updatedPosition);
	object->node->SetOrientation(updatedOrientation);

	// Update velocity and angular velocity
	this->velocity += timestep * avg_acceleration;
	this->angular_velocity += timestep * avg_angular_acceleration;

	// Clear accumulated forces and torques
	this->accum_force = glm::vec3(0.0, 0.0, 0.0);
	this->accum_torque = glm::vec3(0.0, 0.0, 0.0);

	if (canSleep) UpdateKineticEnergyStoreAndPutToSleep(timestep);
}

void RigidBody::AttractTowardsWithPID(float Kp, float Ki, float Kd, const glm::vec3& target)
{
	glm::vec3 position = object->node->GetWorldPosition();
	glm::vec3 error = target - position;

	glm::vec3 derivative = error - prevError;

	glm::vec3 PIDOutput = Kp * error + Ki * integral + Kd * derivative;

	ApplyImpulse(PIDOutput, glm::vec3(0));

	// Update integral and previous error terms
	integral += error;
	prevError = error;
}

void RigidBody::ApplyImpulse(const glm::vec3& force, const glm::vec3& target)
{
	SetAwake();
	this->accum_force += force;
	glm::vec3 position = object->node->GetWorldPosition();
	glm::vec3 directionFromCenterToHitPoint;
	if (target != position) {
		directionFromCenterToHitPoint = glm::normalize(target - position);

		glm::vec3 torque = glm::cross(directionFromCenterToHitPoint, force);
		this->accum_torque += torque;
	}
}

void RigidBody::ApplyImpulse(const glm::vec3& direction, float magnitude, const glm::vec3& target)
{
	SetAwake();
	this->accum_force += direction*magnitude;
	glm::vec3 position = object->node->GetWorldPosition();
	glm::vec3 directionFromCenterToHitPoint;
	if (target != position) {
		directionFromCenterToHitPoint = glm::normalize(target - position);

		glm::vec3 torque = glm::cross(directionFromCenterToHitPoint, direction);
		this->accum_torque += torque * magnitude;
	}
}

void RigidBody::SetInertiaTensor(const glm::mat3& Inertia)
{
	float massSquared = mass * mass;
	glm::mat3 scaled_inertia_tensor = Inertia * massSquared;
	inverse_inertia_tensor = glm::inverse(scaled_inertia_tensor);
	
	//inverse_inertia_tensor = glm::mat3(mass);
	//inverse_inertia_tensor *= Inertia;
	//inverse_inertia_tensor = glm::inverse(inverse_inertia_tensor);


	//inverse_inertia_tensor = (Matrix3::scale(mass, mass, mass)*Inertia).inverse();
}

double RigidBody::GetMass()
{
	return this->mass;
}

double RigidBody::GetMassInverse()
{
	return this->massInverse;
}

void RigidBody::SetMass(double mass)
{
	this->mass = mass;
	if (!isKinematic)	{
		massInverse = 1.0 / mass;
		SetInertiaTensor(MathUtils::CuboidInertiaTensor(object->bounds->obb.extents));
	}
}

Component* RigidBody::Clone()
{
	return new RigidBody(*this);
}

void RigidBody::SetIsKinematic(bool kinematic)
{
	if (kinematic)
	{
		mass = DBL_MAX;
		massInverse = 0.0;
		inverse_inertia_tensor = glm::mat3();
	}
	isKinematic = kinematic;
}

bool RigidBody::GetIsKinematic()
{
	return isKinematic;
}

void RigidBody::UpdateKineticEnergyStoreAndPutToSleep(float timestep)
{
	// Update the kinetic energy store, and possibly put the body to
	// sleep.
	//if (canSleep) {
		double currentMotion = glm::dot(velocity, velocity) + glm::dot(angular_velocity, angular_velocity);
		double bias = pow(0.5, timestep);
		motion = bias*motion + (1.0 - bias)*currentMotion;

		if (motion < sleepEpsilon) SetAwake(false);
		else if (motion > 10.0 * sleepEpsilon) motion = 10.0 * sleepEpsilon;
	//}
}

void RigidBody::UpdateInertiaTensor()
{
	inverse_inertia_tensor_world = object->bounds->obb.rot * inverse_inertia_tensor * (glm::transpose(object->bounds->obb.rot));
}

void RigidBody::SetAwake(const bool awake)
{
	if (awake) {
		isAwake = true;

		// Add a bit of motion to avoid it falling asleep immediately.
		motion = this->sleepEpsilon*2.0;
		object->bounds->obb.color = glm::vec3(0.f, 0.8f, 0.8f);
	}
	else {
		isAwake = false;
		velocity = glm::vec3();
		angular_velocity = glm::vec3();
		object->bounds->obb.color = glm::vec3(2.0f, 0.0f, 0.0f);
	}
}

void RigidBody::SetCanSleep(const bool canSleep)
{
	this->canSleep = canSleep;

	if (!canSleep && !isAwake) SetAwake();
}

void RigidBody::Update()
{
	if (!isAwake || isKinematic) return;
	
	SetMass(mass);
	UpdateInertiaTensor();
	IntegrateRunge(Times::Instance()->timeStep, PhysicsManager::Instance()->gravity);
}