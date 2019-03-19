#include "Bounds.h"
#include "Object.h"
#include <algorithm>
#include <chrono>

using namespace mwm;

const Vector3 Bounds::vertices[8] = {
	Vector3(-0.5, -0.5, 0.5),
	Vector3(0.5, -0.5, 0.5),
	Vector3(0.5, 0.5, 0.5),
	Vector3(-0.5, 0.5, 0.5),

	Vector3(-0.5, -0.5, -0.5),
	Vector3(0.5, -0.5, -0.5),
	Vector3(0.5, 0.5, -0.5),
	Vector3(-0.5, 0.5, -0.5)
};

double Bounds::updateBoundsTime;
double Bounds::updateMinMaxTime;


Bounds::Bounds(){
	radius = 0.5;
	name = "cube";
	dimensions = Vector3(1.0, 1.0, 1.0);
	centerOfMesh = Vector3();
	obb.color = Vector3F(0.f, 0.8f, 0.8f);
	aabb.color = Vector3F(1.f, 0.54f, 0.f);
	MeshCenterM.setIdentity();
	centeredPosition.zero();
	CenteredTopDownTransform.setIdentity();
	dynamic = true;
}

void Bounds::Update()
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;

	start = std::chrono::high_resolution_clock::now();
	CenteredTopDownTransform = MeshCenterM * object->node->TopDownTransform;
	centeredPosition = CenteredTopDownTransform.getPosition();
	obb.extents = dimensions * object->node->totalScale;
	obb.halfExtents = obb.extents*0.5;
	
	radius = std::max(std::max(obb.halfExtents.x, obb.halfExtents.y), obb.halfExtents.z); //perfect for sphere
	circumRadius = obb.halfExtents.vectLengt(); //perfect for cuboid

	obb.rot = CenteredTopDownTransform.extractRotation3(); //no scaling
	obb.model.setIdentity();
	obb.model.setScale(dimensions);
	obb.model *= CenteredTopDownTransform; //total matrix contain scale of the object which applies to the bounding box that is 1 unit large, we need to scale the bb up to so it matches the dimensions of the mesh it will surround
	//obb.model = Matrix4::scale(dimensions)*CenteredTopDownTransform; //total matrix contain scale of the object which applies to the bounding box that is 1 unit large, we need to scale the bb up to so it matches the dimensions of the mesh it will surround
	
	
	std::chrono::time_point<std::chrono::high_resolution_clock> startMinMax, endMinMax;
	startMinMax = std::chrono::high_resolution_clock::now();
	UpdateMinMax(obb.model.ConvertToMatrix3(), centeredPosition); //we could just send the obb.model(4x4) but 3x3 + pos is faster

	endMinMax = std::chrono::high_resolution_clock::now();
	elapsed_seconds = endMinMax - startMinMax;
	updateMinMaxTime += elapsed_seconds.count();

	aabb.extents = obb.mm.max - obb.mm.min;
	aabb.model.setScale(aabb.extents);
	aabb.model.setPosition(centeredPosition);

	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	updateBoundsTime += elapsed_seconds.count();
}

void Bounds::SetBoundsCenter(const mwm::Vector3 & center)
{
	MeshCenterM.setPosition(center);
	centerOfMesh = center;
}

void Bounds::SetBoundsDimensions(const mwm::Vector3 & newDimensions)
{
	dimensions = newDimensions;
}

mwm::Vector3 Bounds::GetBoundsCenter()
{
	return centerOfMesh;
}

mwm::Vector3 Bounds::GetBoundsDimensions()
{
	return dimensions;
}

std::string Bounds::GetMeshName()
{
	return name;
}

mwm::Vector3 Bounds::GetExtents()
{
	return obb.extents;
}

void Bounds::SetUp(Vector3& newCenter, Vector3& newDimensions, std::string& newName)
{
	MeshCenterM.setPosition(newCenter);
	centerOfMesh = newCenter;
	dimensions = newDimensions;
	name = newName;
}

void Bounds::UpdateMinMax(const Matrix3& modelM, const Vector3& position)
{
	obb.mm.max = modelM * vertices[0];
	obb.mm.min = obb.mm.max;
	Vector3 currentVertex;
	for (int i = 0; i < 8; ++i)
	{
		currentVertex = modelM * vertices[i];
		if (currentVertex.x > obb.mm.max.x) obb.mm.max.x = currentVertex.x;
		if (currentVertex.y > obb.mm.max.y) obb.mm.max.y = currentVertex.y;
		if (currentVertex.z > obb.mm.max.z) obb.mm.max.z = currentVertex.z;

		if (currentVertex.x < obb.mm.min.x) obb.mm.min.x = currentVertex.x;
		if (currentVertex.y < obb.mm.min.y) obb.mm.min.y = currentVertex.y;
		if (currentVertex.z < obb.mm.min.z) obb.mm.min.z = currentVertex.z;
	}
	obb.mm.max += position;
	obb.mm.min += position;
}

void Bounds::UpdateMinMax(const mwm::Matrix4 & modelMatrix)
{
	Vector4 maxValuesW = modelMatrix * vertices[0];
	Vector4 minValuesW = maxValuesW;
	Vector4 currentVertex;
	for (int i = 0; i < 8; ++i)
	{
		currentVertex = modelMatrix * vertices[i];
		if (currentVertex.x > obb.mm.max.x) obb.mm.max.x = currentVertex.x;
		if (currentVertex.y > obb.mm.max.y) obb.mm.max.y = currentVertex.y;
		if (currentVertex.z > obb.mm.max.z) obb.mm.max.z = currentVertex.z;

		if (currentVertex.x < obb.mm.min.x) obb.mm.min.x = currentVertex.x;
		if (currentVertex.y < obb.mm.min.y) obb.mm.min.y = currentVertex.y;
		if (currentVertex.z < obb.mm.min.z) obb.mm.min.z = currentVertex.z;
	}
	obb.mm.max = maxValuesW.get_xyz();
	obb.mm.min = minValuesW.get_xyz();
}