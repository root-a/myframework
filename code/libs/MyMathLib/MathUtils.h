#pragma once
#include "Vector3.h"
#include "Vector3F.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class MathUtils
{
public:
	MathUtils();
	~MathUtils();
	static double PI;
	static Vector3 ToRadians(const Vector3& degVec);
	static Vector3F ToRadians(const Vector3F& degVec);
	static Vector3 ToDegrees(const Vector3& degVec);
	static Vector3F ToDegrees(const Vector3F& degVec);
	static double ToRadians(double degrees);
	static double ToDegrees(double radians);
	static glm::mat4 CalculateRelativeTransform(const glm::mat4& parent, const glm::mat4& child);
	static void SetPosition(glm::mat4& mat, const glm::vec3& pos);
	static glm::vec3 GetPosition(const glm::mat4& mat);
	static glm::mat3 ExtractRotation(const glm::mat4& mat);
	static glm::vec3 ExtractScale(const glm::mat4& mat);
	static void SetScale(glm::mat4& mat, const glm::vec3& scale);
	static void ZeroPosition(glm::mat4& mat);
	static glm::vec3 GetForward(const glm::mat4& mat);
	static glm::vec3 GetForward(const glm::mat3& mat);
	static glm::mat4 BiasMatrix();
	static glm::vec3 GetAxis(const glm::mat3& mat, int axis);
	static glm::mat3 CuboidInertiaTensor(const glm::vec3& dimensions);
private:

};