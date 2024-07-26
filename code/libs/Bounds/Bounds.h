#pragma once
#include "MyMathLib.h"
#include "OBBAABB.h"
#include <string>
#include "Component.h"

class Bounds : public Component
{
	
public:
	Bounds();
	Bounds(const glm::vec3& Center, const glm::vec3& Dimensions, const std::string& Name);

	void Update();
	void Init(Object* parent);
	glm::mat4 CenteredTopDownTransform;
	glm::mat4 MeshCenterM;
	glm::vec3 centeredPosition;
	AABB aabb;
	OBB obb;
	double radius;
	double circumRadius;
	glm::vec3 centerOfMesh;
	glm::vec3 dimensions;
	std::string name;
	void SetBoundsCenter(const glm::vec3& center);
	void SetBoundsDimensions(const glm::vec3& dimensions);
	glm::vec3 GetBoundsCenter();
	glm::vec3 GetBoundsDimensions();
	std::string& GetMeshName();
	glm::vec3 GetExtents();
	void SetUp(const glm::vec3& center, const glm::vec3& dimensions, const std::string& name);
	void UpdateMinMax(const glm::mat3& modelMatrix, const glm::vec3& position);
	void UpdateMinMax(const glm::mat4& modelMatrix, const glm::vec3& position);
	Component* Clone();
	static const glm::vec3 vertices[8];
	static double updateBoundsTime;
	static double updateMinMaxTime;
	glm::vec3 currentVertex;
};
