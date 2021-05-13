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
	void Init(Object* parent);
	Matrix4 CenteredTopDownTransform;
	Matrix4 MeshCenterM;
	Vector3 centeredPosition;
	AABB aabb;
	OBB obb;
	double radius;
	double circumRadius;
	Vector3 centerOfMesh;
	Vector3 dimensions;
	std::string name;
	void SetBoundsCenter(const Vector3 & center);
	void SetBoundsDimensions(const Vector3 & dimensions);
	Vector3 GetBoundsCenter();
	Vector3 GetBoundsDimensions();
	std::string& GetMeshName();
	Vector3 GetExtents();
	void SetUp(Vector3& center, Vector3& dimensions, std::string& name);
	void UpdateMinMax(const Matrix3& modelMatrix, const Vector3& position);
	void UpdateMinMax(const Matrix4& modelMatrix, const Vector3& position);
	void UpdateMinMax(const Matrix4& modelMatrix);
	
	static const Vector3 vertices[8];
	static double updateBoundsTime;
	static double updateMinMaxTime;
	Vector3 currentVertex;
};
