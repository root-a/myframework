#include "RigidBody.h"
#include <cmath> 
#include "Object.h"
#include "PhysicsManager.h"
#include "Time.h"
#include "DebugDraw.h"

using namespace mwm;

RigidBody::RigidBody(Object* parent)
{
	this->obb.color = Vector3(0.f, 0.8f, 0.8f);
	this->aabb.color = Vector3(1.f, 0.54f, 0.f);
	this->mass = 1.f;
	this->massInverse = 1.f / 1.f;
	this->damping = 0.85f;
	this->isAwake = true;
	this->isKinematic = false;
	this->restitution = 0.f;
	this->motion = 1.f; //make sure it does not sleep directly at start of simulation
	this->canSleep = true;
	this->sleepEpsilon = 0.2f;
	this->object = parent;
	UpdateHExtentsAndMass();
}

RigidBody::~RigidBody()
{
}

void RigidBody::IntegrateEuler(float timestep, const Vector3& gravity)
{
	if (!isAwake || isKinematic) return;

	this->acceleration = gravity + this->accum_force * this->massInverse;

	this->angular_acc = inverse_inertia_tensor_world*this->accum_torque;

	//acceleration is 
	//change in velocity / change in time
	Vector3 changeInVel = acceleration * timestep;
	this->velocity += changeInVel;

	this->angular_velocity += angular_acc * timestep;

	this->velocity *= pow(damping, timestep);
	this->angular_velocity *= pow(damping, timestep);

	Vector3 changeInPos = this->velocity * timestep;

	object->Translate(changeInPos);

	Vector3 axis = this->angular_velocity.vectNormalize();
	float angle = this->angular_velocity.vectLengt();
	if (angle != 0){
		Quaternion test(angle * timestep, axis);
		//angularvel * quaternion
		object->SetOrientation((test*object->GetOrientation()).Normalized());
	}

	//clear force
	this->accum_force = Vector3(0.f, 0.f, 0.f);
	this->accum_torque = Vector3(0.f, 0.f, 0.f);

	if (canSleep) UpdateKineticEnergyStoreAndPutToSleep(timestep);
}

void RigidBody::IntegrateMid(float timestep, const Vector3& gravity)
{
	if (!isAwake || isKinematic) return;

	this->acceleration = gravity + this->accum_force * this->massInverse;

	this->angular_acc = inverse_inertia_tensor_world*this->accum_torque;

	//acceleration is 
	//change in velocity / change in time
	Vector3 changeInVel = acceleration * (timestep * 0.5f); //half-step
	Vector3 changeInVel2 = (acceleration + changeInVel) * timestep; // midpoint result
	this->velocity += changeInVel2;

	Vector3 changeInAngVel = angular_acc * (timestep * 0.5f); //half-step
	Vector3 changeInAngVel2 = (angular_acc + changeInAngVel) * timestep; // midpoint result
	this->angular_velocity += changeInAngVel2;

	this->velocity *= pow(damping, timestep);
	this->angular_velocity *= pow(damping, timestep);

	//midpoint	
	Vector3 changeInPos = this->velocity*timestep;

	object->Translate(changeInPos);

	Vector3 axis = this->angular_velocity.vectNormalize();
	float angle = this->angular_velocity.vectLengt();
	if (angle != 0){
		Quaternion test(angle * timestep, axis);
		object->SetOrientation((test*object->GetOrientation()).Normalized());
	}

	//clear force
	this->accum_force = Vector3(0.f, 0.f, 0.f);
	this->accum_torque = Vector3(0.f, 0.f, 0.f);

	if (canSleep) UpdateKineticEnergyStoreAndPutToSleep(timestep);
}

void RigidBody::IntegrateRunge(float timestep, const Vector3& gravity)
{
	if (!isAwake || isKinematic) return;

	this->acceleration = gravity + this->accum_force * this->massInverse;

	this->angular_acc = inverse_inertia_tensor_world*this->accum_torque;

	Vector3 dx1 = velocity;
	Vector3 dv1 = acceleration;
	// step 2
	Vector3 dx2 = velocity + (timestep / 2.0f) * dx1;
	Vector3 dv2 = acceleration + (timestep / 2.0f) * dv1; //k2 vel
	// step 3
	Vector3 dx3 = velocity + (timestep / 2.0f) * dx2;
	Vector3 dv3 = acceleration + (timestep / 2.0f) * dv2; //k3 vel
	// step 4
	Vector3 dx4 = velocity + timestep * dx3;
	Vector3 dv4 = acceleration + timestep * dv3; //k4 vel
	// now combine the derivative estimates and
	// compute new state
	Vector3 changeInPos = (timestep / 6.0f) * (dx1 + (dx2 + dx3) * 2.0f + dx4);
	object->Translate(changeInPos);

	this->velocity += (timestep / 6.0f) * (dv1 + (dv2 + dv3) * 2.0f + dv4);
	this->velocity *= pow(damping, timestep);

	//orientation
	Vector3 adx1 = angular_velocity;
	Vector3 adv1 = angular_acc;
	// step 2
	Vector3 adx2 = angular_velocity + (timestep / 2.0f) * adx1;
	Vector3 adv2 = angular_acc + (timestep / 2.0f) * adv1; //k2 ang
	// step 3
	Vector3 adx3 = angular_velocity + (timestep / 2.0f) * adx2;
	Vector3 adv3 = angular_acc + (timestep / 2.0f) * adv2; //k3 ang
	// step 4
	Vector3 adx4 = angular_velocity + timestep * adx3;
	Vector3 adv4 = angular_acc + timestep * adv3; //k4 ang
	// now combine the derivative estimates and
	// compute new state

	Vector3 changeInRot = (timestep / 6.0f) * (adx1 + (adx2 + adx3) * 2.0f + adx4);

	this->angular_velocity += (timestep / 6.0f) * (adv1 + adv2 * 2.0f + adv3 * 2.0f + adv4);
	this->angular_velocity *= pow(damping, timestep);

	Vector3 axis = changeInRot.vectNormalize();
	float angle = changeInRot.vectLengt();
	if (angle != 0){
		Quaternion test(angle, axis);
		object->SetOrientation((test*object->GetOrientation()).Normalized());
	}

	//clear force
	this->accum_force = Vector3(0.f, 0.f, 0.f);
	this->accum_torque = Vector3(0.f, 0.f, 0.f);

	if (canSleep) UpdateKineticEnergyStoreAndPutToSleep(timestep);
}

void RigidBody::ApplyImpulse(const Vector3& force, const Vector3& picking_point)
{
	SetAwake();
	this->accum_force += force;
	//DebugDraw::Instance()->DrawShapeAtPos("cube", picking_point);
	//DebugDraw::Instance()->DrawShapeAtPos("pyramid", picking_point+force);
	Vector3 directionFromCenterToPickingPoint = picking_point - object->GetWorldPosition();
	Vector3 torque = directionFromCenterToPickingPoint.crossProd(force);
	this->accum_torque += torque;
}

void RigidBody::ApplyImpulse(const Vector3& direction, float magnitude, const Vector3& point)
{
	SetAwake();
	this->accum_force += direction*magnitude;
	//DebugDraw::Instance()->DrawShapeAtPos("cube", point);
	//DebugDraw::Instance()->DrawShapeAtPos("pyramid", picking_point+force);
	Vector3 directionFromCenterToPickingPoint = point - object->GetWorldPosition();
	Vector3 torque = directionFromCenterToPickingPoint.crossProd(direction);
	this->accum_torque += torque*magnitude;
}

void RigidBody::SetInertiaTensor(const Matrix3& I)
{
	this->inverse_inertia_tensor = I.inverse();
}

float RigidBody::GetMass()
{
	return this->mass;
}

float RigidBody::GetMassInverse()
{
	return this->massInverse;
}

void RigidBody::SetMass(float mass)
{
	this->mass = mass;
	if (mass < FLT_MAX)	{
		this->massInverse = 1.f / mass;
		SetInertiaTensor(Matrix3::CuboidInertiaTensor(this->mass, object->GetMeshDimensions()*this->object->getScale()));
	}
	else {
		this->massInverse = 0.f;
		inverse_inertia_tensor = Matrix3();
	}
}

void RigidBody::UpdateKineticEnergyStoreAndPutToSleep(float timestep)
{
	// Update the kinetic energy store, and possibly put the body to
	// sleep.
	if (canSleep) {

		float currentMotion = velocity.dotAKAscalar(velocity) + angular_velocity.dotAKAscalar(angular_velocity);
		float bias = powf(0.5f, timestep);
		motion = bias*motion + (1.f - bias)*currentMotion;

		if (motion < sleepEpsilon) SetAwake(false);
		else if (motion > 10.f * sleepEpsilon) motion = 10.f * sleepEpsilon;
	}
}

void RigidBody::UpdateBoundingBoxes(const BoundingBox& boundingBox)
{
	//OBB
	obb.model = object->node.orientation.ConvertToMatrix3(); //don't let the model contain the scale
	obb.mm = boundingBox.CalcValuesInWorld(extentsM * obb.model, object->node.position);

	//AABB
	aabb.model = Matrix4::scale((obb.mm.max - obb.mm.min))*Matrix4::translate(object->node.position);
}

void RigidBody::UpdateInertiaTensor()
{
	inverse_inertia_tensor_world = obb.model * this->inverse_inertia_tensor * (~obb.model);
}

void RigidBody::SetOBBHalfExtent(const Vector3& scale)
{
	obb.halfExtent = scale;
}

void RigidBody::SetAwake(const bool awake)
{
	if (awake) {
		isAwake = true;

		// Add a bit of motion to avoid it falling asleep immediately.
		motion = this->sleepEpsilon*2.0f;
		this->obb.color = Vector3(0.f, 0.8f, 0.8f);
	}
	else {
		isAwake = false;
		velocity = Vector3(0.f, 0.f, 0.f);
		angular_velocity = Vector3(0.f, 0.f, 0.f);
		this->obb.color = Vector3(2.0f, 0.0f, 0.0f);
	}
}

void RigidBody::SetCanSleep(const bool canSleep)
{
	this->canSleep = canSleep;

	if (!canSleep && !isAwake) SetAwake();
}

void RigidBody::Update()
{
	IntegrateRunge((float)Time::timeStep, PhysicsManager::Instance()->gravity);
	UpdateBoundingBoxes(DebugDraw::Instance()->boundingBox);
	UpdateInertiaTensor();
}

void RigidBody::UpdateHalfExtents()
{
	Vector3 extents = object->GetMeshDimensions()*object->getScale();
	extentsM = Matrix3::scale(extents);
	obb.halfExtent = extents*0.5f;
}

void RigidBody::UpdateHExtentsAndMass()
{
	UpdateHalfExtents();
	SetMass(this->mass);
}
