#include "Bounds.h"
#include "Object.h"
#include <algorithm>
#include <chrono>

const glm::vec3 Bounds::vertices[8] = {
	glm::vec3(-0.5, -0.5, 0.5),
	glm::vec3(0.5, -0.5, 0.5),
	glm::vec3(0.5, 0.5, 0.5),
	glm::vec3(-0.5, 0.5, 0.5),

	glm::vec3(-0.5, -0.5, -0.5),
	glm::vec3(0.5, -0.5, -0.5),
	glm::vec3(0.5, 0.5, -0.5),
	glm::vec3(-0.5, 0.5, -0.5)
};

double Bounds::updateBoundsTime;
double Bounds::updateMinMaxTime;

Bounds::Bounds()
{
	radius = 0.5;
	name = "none";
	dimensions = glm::vec3(1,1,1);
	centerOfMesh = glm::vec3();
	MeshCenterM = glm::mat4(1);
	MathUtils::SetPosition(MeshCenterM, centerOfMesh);
	obb.color = glm::vec3(0.f, 0.8f, 0.8f);
	aabb.color = glm::vec3(1.f, 0.54f, 0.f);
	centeredPosition = glm::vec3(0);
	CenteredTopDownTransform = glm::mat4(1);
}

Bounds::Bounds(const glm::vec3& Center, const glm::vec3& Dimensions, const std::string& Name){
	radius = 0.5;
	name = Name;
	dimensions = Dimensions;
	centerOfMesh = Center;
	MeshCenterM = glm::mat4(1);;
	MathUtils::SetPosition(MeshCenterM, Center);
	obb.color = glm::vec3(0.f, 0.8f, 0.8f);
	aabb.color = glm::vec3(1.f, 0.54f, 0.f);
	centeredPosition = glm::vec3(0);
	CenteredTopDownTransform = glm::mat4(1);
}

void Bounds::Init(Object * parent)
{
	Component::Init(parent);

	parent->bounds = this;
}

void Bounds::Update()
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;

	start = std::chrono::high_resolution_clock::now();
	CenteredTopDownTransform = MeshCenterM * object->node->TopDownTransform;
	centeredPosition = MathUtils::GetPosition(CenteredTopDownTransform);
	obb.extents = dimensions * object->node->totalScale;
	obb.halfExtents = obb.extents*0.5f;
	
	radius = std::max(std::max(obb.halfExtents.x, obb.halfExtents.y), obb.halfExtents.z); //perfect for sphere, radius around geometry
	circumRadius = glm::length(obb.halfExtents); //perfect for cuboid, radius inside geometry

	obb.rot = MathUtils::ExtractRotation(CenteredTopDownTransform); //no scaling
	obb.model = glm::mat4(1);
	MathUtils::SetScale(obb.model, dimensions);
	obb.model = CenteredTopDownTransform * obb.model; //total matrix contain scale of the object which applies to the bounding box that is 1 unit large, we need to scale the bb up to so it matches the dimensions of the mesh it will surround
	//obb.model = Matrix4::scale(dimensions)*CenteredTopDownTransform; //total matrix contain scale of the object which applies to the bounding box that is 1 unit large, we need to scale the bb up to so it matches the dimensions of the mesh it will surround
	
	
	std::chrono::time_point<std::chrono::high_resolution_clock> startMinMax, endMinMax;
	startMinMax = std::chrono::high_resolution_clock::now();
	UpdateMinMax(obb.model, centeredPosition); //we could just send the obb.model(4x4) but 3x3 + pos is faster

	endMinMax = std::chrono::high_resolution_clock::now();
	elapsed_seconds = endMinMax - startMinMax;
	updateMinMaxTime += elapsed_seconds.count();

	aabb.extents = obb.mm.max - obb.mm.min;
	MathUtils::SetScale(aabb.model, aabb.extents);
	MathUtils::SetPosition(aabb.model, centeredPosition);

	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	updateBoundsTime += elapsed_seconds.count();
}

void Bounds::SetBoundsCenter(const glm::vec3& center)
{
	MathUtils::SetPosition(MeshCenterM, center);
	centerOfMesh = center;
}

void Bounds::SetBoundsDimensions(const glm::vec3& newDimensions)
{
	dimensions = newDimensions;
}

glm::vec3 Bounds::GetBoundsCenter()
{
	return centerOfMesh;
}

glm::vec3 Bounds::GetBoundsDimensions()
{
	return dimensions;
}

std::string& Bounds::GetMeshName()
{
	return name;
}

glm::vec3 Bounds::GetExtents()
{
	return obb.extents;
}

void Bounds::SetUp(const glm::vec3& newCenter, const glm::vec3& newDimensions, const std::string& newName)
{
	MathUtils::SetPosition(MeshCenterM, newCenter);
	centerOfMesh = newCenter;
	dimensions = newDimensions;
	name = newName;
}

void Bounds::UpdateMinMax(const glm::mat3& modelM, const glm::vec3& position)
{
	currentVertex = modelM * vertices[0];
	obb.mm.max = currentVertex;
	obb.mm.min = currentVertex;

	if (currentVertex.x > obb.mm.max.x) obb.mm.max.x = currentVertex.x;
	if (currentVertex.y > obb.mm.max.y) obb.mm.max.y = currentVertex.y;
	if (currentVertex.z > obb.mm.max.z) obb.mm.max.z = currentVertex.z;

	if (currentVertex.x < obb.mm.min.x) obb.mm.min.x = currentVertex.x;
	if (currentVertex.y < obb.mm.min.y) obb.mm.min.y = currentVertex.y;
	if (currentVertex.z < obb.mm.min.z) obb.mm.min.z = currentVertex.z;

	for (int i = 1; i < 8; ++i)
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

void Bounds::UpdateMinMax(const glm::mat4& modelM, const glm::vec3& position)
{
	UpdateMinMax(glm::mat3(modelM), position);
}

Component* Bounds::Clone()
{
	return new Bounds(*this);
}
