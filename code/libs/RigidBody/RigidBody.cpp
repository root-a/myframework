#include "RigidBody.h"
#include <cmath> 
#include "Object.h"
#include "PhysicsManager.h"
#include "Times.h"
#include "BoundingBox.h"

using namespace mwm;

RigidBody::RigidBody(Object* owner)
{
	this->obb.color = Vector3F(0.f, 0.8f, 0.8f);
	this->aabb.color = Vector3F(1.f, 0.54f, 0.f);
	this->mass = 1.0;
	this->massInverse = 1.0 / 1.0;
	this->angularDamping = 0.85;
	this->linearDamping = 0.85;
	this->isAwake = true;
	this->isKinematic = false;
	this->restitution = 0.0;
	this->motion = 1.0; //make sure it does not sleep directly at start of simulation
	this->canSleep = true;
	this->sleepEpsilon = 0.2;
	this->object = owner;
	UpdateHExtentsAndMass();
}

RigidBody::~RigidBody()
{
}

void RigidBody::IntegrateEuler(double timestep, const Vector3& gravity)
{
	if (!isAwake || isKinematic) return;

	this->acceleration = gravity + this->accum_force * this->massInverse;
	this->angular_acc = inverse_inertia_tensor_world*this->accum_torque;

	Vector3 changeInVel = acceleration * timestep;
	this->velocity += changeInVel;

	Vector3 changeInPos = this->velocity * timestep;
	object->Translate(changeInPos);

	this->velocity *= pow(linearDamping, timestep);


	this->angular_velocity += angular_acc * timestep;

	double angle = this->angular_velocity.vectLengt();
	Vector3 axis = this->angular_velocity / angle;
	if (angle != 0){
		Quaternion test(MathUtils::ToDegrees(angle) * timestep, axis);
		object->SetOrientation((test*object->GetLocalOrientation()).Normalized());
	}

	this->angular_velocity *= pow(angularDamping, timestep);

	this->accum_force = Vector3(0.0, 0.0, 0.0);
	this->accum_torque = Vector3(0.0, 0.0, 0.0);

	if (canSleep) UpdateKineticEnergyStoreAndPutToSleep(timestep);
}

void RigidBody::IntegrateMid(double timestep, const Vector3& gravity)
{
	if (!isAwake || isKinematic) return;

	this->acceleration = gravity + this->accum_force * this->massInverse;
	this->angular_acc = inverse_inertia_tensor_world*this->accum_torque;

	Vector3 changeInVel = acceleration * (timestep * 0.5); //half-step
	Vector3 changeInVel2 = (acceleration + changeInVel) * timestep; // midpoint result

	this->velocity += changeInVel2;

	Vector3 changeInPos = this->velocity*timestep;
	object->Translate(changeInPos);

	this->velocity *= pow(linearDamping, timestep);


	Vector3 changeInAngVel = angular_acc * (timestep * 0.5); //half-step
	Vector3 changeInAngVel2 = (angular_acc + changeInAngVel) * timestep; // midpoint result

	this->angular_velocity += changeInAngVel2;

	double angle = this->angular_velocity.vectLengt();
	Vector3 axis = this->angular_velocity / angle;
	if (angle != 0){
		Quaternion test(MathUtils::ToDegrees(angle) * timestep, axis);
		object->SetOrientation((test*object->GetLocalOrientation()).Normalized());
	}

	this->angular_velocity *= pow(angularDamping, timestep);

	this->accum_force = Vector3(0.0, 0.0, 0.0);
	this->accum_torque = Vector3(0.0, 0.0, 0.0);

	if (canSleep) UpdateKineticEnergyStoreAndPutToSleep(timestep);
}

void RigidBody::IntegrateRunge(double timestep, const Vector3& gravity)
{
	if (!isAwake || isKinematic) return;

	this->acceleration = gravity + this->accum_force * this->massInverse;
	this->angular_acc = inverse_inertia_tensor_world*this->accum_torque;

	Vector3 dv1 = acceleration;
	Vector3 dv2 = acceleration + (timestep / 2.0) * dv1; //k2 vel
	Vector3 dv3 = acceleration + (timestep / 2.0) * dv2; //k3 vel
	Vector3 dv4 = acceleration + timestep * dv3; //k4 vel

	this->velocity += (timestep / 6.0) * (dv1 + (dv2 + dv3) * 2.0 + dv4);

	Vector3 dx1 = velocity;
	Vector3 dx2 = velocity + (timestep / 2.0) * dx1;
	Vector3 dx3 = velocity + (timestep / 2.0) * dx2;
	Vector3 dx4 = velocity + timestep * dx3;

	Vector3 changeInPos = (timestep / 6.0) * (dx1 + (dx2 + dx3) * 2.0 + dx4);
	object->Translate(changeInPos);

	this->velocity *= pow(linearDamping, timestep);
	
	Vector3 adv1 = angular_acc;
	Vector3 adv2 = angular_acc + (timestep / 2.0) * adv1; //k2 ang
	Vector3 adv3 = angular_acc + (timestep / 2.0) * adv2; //k3 ang
	Vector3 adv4 = angular_acc + timestep * adv3; //k4 ang

	this->angular_velocity += (timestep / 6.0) * (adv1 + adv2 * 2.0 + adv3 * 2.0 + adv4);

	Vector3 adx1 = angular_velocity;
	Vector3 adx2 = angular_velocity + (timestep / 2.0) * adx1;
	Vector3 adx3 = angular_velocity + (timestep / 2.0) * adx2;
	Vector3 adx4 = angular_velocity + timestep * adx3;

	Vector3 changeInRot = (timestep / 6.0) * (adx1 + (adx2 + adx3) * 2.0 + adx4);

	double angle = changeInRot.vectLengt();
	Vector3 axis = changeInRot / angle; //normalize
	if (angle != 0){
		Quaternion test(MathUtils::ToDegrees(angle), axis);
		object->SetOrientation((test*object->GetLocalOrientation()).Normalized());
	}

	this->angular_velocity *= pow(angularDamping, timestep);

	this->accum_force = Vector3(0.0, 0.0, 0.0);
	this->accum_torque = Vector3(0.0, 0.0, 0.0);

	if (canSleep) UpdateKineticEnergyStoreAndPutToSleep(timestep);
}

void RigidBody::ApplyImpulse(const Vector3& force, const Vector3& target)
{
	SetAwake();
	this->accum_force += force;
	//DebugDraw::Instance()->DrawShapeAtPos("cube", picking_point);
	//DebugDraw::Instance()->DrawShapeAtPos("pyramid", picking_point+force);
	Vector3 directionFromCenterToPickingPoint = (target - object->GetWorldPosition()).vectNormalize();
	Vector3 torque = directionFromCenterToPickingPoint.crossProd(force);
	this->accum_torque += torque;
}

void RigidBody::ApplyImpulse(const Vector3& direction, double magnitude, const Vector3& target)
{
	SetAwake();
	this->accum_force += direction*magnitude;
	//DebugDraw::Instance()->DrawShapeAtPos("cube", point);
	//DebugDraw::Instance()->DrawShapeAtPos("pyramid", picking_point+force);
	Vector3 directionFromCenterToPickingPoint = (target - object->GetWorldPosition()).vectNormalize();
	Vector3 torque = directionFromCenterToPickingPoint.crossProd(direction);
	this->accum_torque += torque*magnitude;
}

void RigidBody::SetInertiaTensor(const Matrix3& I)
{
	this->inverse_inertia_tensor = I.inverse();
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
	if (mass < FLT_MAX)	{
		this->massInverse = 1.f / mass;
		SetInertiaTensor(Matrix3::CuboidInertiaTensor(this->mass, object->GetMeshDimensions()*this->object->getScale()));
	}
	else {
		this->massInverse = 0.f;
		inverse_inertia_tensor = Matrix3();
	}
}

void RigidBody::UpdateKineticEnergyStoreAndPutToSleep(double timestep)
{
	// Update the kinetic energy store, and possibly put the body to
	// sleep.
	if (canSleep) {

		double currentMotion = velocity.dotAKAscalar(velocity) + angular_velocity.dotAKAscalar(angular_velocity);
		double bias = pow(0.5, timestep);
		motion = bias*motion + (1.0 - bias)*currentMotion;

		if (motion < sleepEpsilon) SetAwake(false);
		else if (motion > 10.0 * sleepEpsilon) motion = 10.0 * sleepEpsilon;
	}
}

void RigidBody::UpdateBoundingBoxes()
{
	obb.rot = object->node.CenteredTopDownTransform.extractRotation3(); //no scaling

	obb.model = Matrix4::scale(object->GetMeshDimensions())*object->node.CenteredTopDownTransform; //total matrix contain scale of the object which applies to the bounding box that is 1 unit large, we need to scale the bb up to so it matches the dimensions of the mesh it will surround
	obb.mm = BoundingBox::CalcValuesInWorld(obb.model.ConvertToMatrix3(), object->node.centeredPosition); //we could just send the obb.model(4x4) but 3x3 + pos is faster
	
	Vector3 aaBBextents = obb.mm.max - obb.mm.min;
	aabb.model = Matrix4::scale(aaBBextents)*Matrix4::translate(object->node.centeredPosition);
}

void RigidBody::UpdateInertiaTensor()
{
	inverse_inertia_tensor_world = obb.rot * this->inverse_inertia_tensor * (~obb.rot);
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
		motion = this->sleepEpsilon*2.0;
		this->obb.color = Vector3F(0.f, 0.8f, 0.8f);
	}
	else {
		isAwake = false;
		velocity = Vector3(0.0, 0.0, 0.0);
		angular_velocity = Vector3(0.0, 0.0, 0.0);
		this->obb.color = Vector3F(2.0f, 0.0f, 0.0f);
	}
}

void RigidBody::SetCanSleep(const bool canSleep)
{
	this->canSleep = canSleep;

	if (!canSleep && !isAwake) SetAwake();
}

void RigidBody::Update()
{
	IntegrateRunge(Times::Instance()->timeStep, PhysicsManager::Instance()->gravity);
	UpdateBoundingBoxes();
	UpdateInertiaTensor();
	UpdateHalfExtents();
}

void RigidBody::UpdateHalfExtents()
{
	obb.halfExtent = object->GetMeshDimensions()*object->getScale()*0.5;
}

void RigidBody::UpdateHExtentsAndMass()
{
	UpdateHalfExtents();
	SetMass(this->mass);
}
