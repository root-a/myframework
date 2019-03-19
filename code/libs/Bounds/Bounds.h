#pragma once
#include "MyMathLib.h"
#include "OBBAABB.h"
#include <string>
#include "Component.h"

class Bounds : public Component
{
	
public:
	Bounds();
	void Update();
	mwm::Matrix4 CenteredTopDownTransform;
	mwm::Matrix4 MeshCenterM;
	mwm::Vector3 centeredPosition;
	mwm::AABB aabb;
	mwm::OBB obb;
	double radius;
	double circumRadius;
	mwm::Vector3 centerOfMesh;
	mwm::Vector3 dimensions;
	std::string name;
	void SetBoundsCenter(const mwm::Vector3 & center);
	void SetBoundsDimensions(const mwm::Vector3 & dimensions);
	mwm::Vector3 GetBoundsCenter();
	mwm::Vector3 GetBoundsDimensions();
	std::string GetMeshName();
	mwm::Vector3 GetExtents();
	void SetUp(mwm::Vector3& center, mwm::Vector3& dimensions, std::string& name);
	void UpdateMinMax(const mwm::Matrix3& modelMatrix, const mwm::Vector3& position);
	void UpdateMinMax(const mwm::Matrix4& modelMatrix);
	
	static const mwm::Vector3 vertices[8];
	static double updateBoundsTime;
	static double updateMinMaxTime;
};
