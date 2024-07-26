#include "MathUtils.h"
#include "Vector3.h"
#include "Vector3F.h"
#include "Matrix4F.h"

MathUtils::MathUtils()
{
}

MathUtils::~MathUtils()
{
}

Vector3
MathUtils::ToRadians(const Vector3& degVec)
{
	return Vector3((degVec.x * PI) / 180.0, (degVec.y * PI) / 180.0, (degVec.z * PI) / 180.0);
}

Vector3F
MathUtils::ToRadians(const Vector3F& degVec)
{
	return Vector3F((degVec.x * PI) / 180.0f, (degVec.y * PI) / 180.0f, (degVec.z * PI) / 180.0f);
}

Vector3
MathUtils::ToDegrees(const Vector3& radVec)
{
	return Vector3((radVec.x * 180.0) / PI, (radVec.y * 180.0) / PI, (radVec.z * 180.0) / PI);
}

Vector3F
MathUtils::ToDegrees(const Vector3F& radVec)
{
	return Vector3F((radVec.x * 180.0f) / PI, (radVec.y * 180.0f) / PI, (radVec.z * 180.0f) / PI);
}

double
MathUtils::ToRadians(double degrees)
{
	return (degrees * PI) / 180.0;
}

double
MathUtils::ToDegrees(double radians)
{
	return (radians * 180.0) / PI;
}

glm::mat4
MathUtils::CalculateRelativeTransform(const glm::mat4& parent, const glm::mat4& child)
{
	return child * glm::inverse(parent);
}

void
MathUtils::SetPosition(glm::mat4& mat, const glm::vec3& pos)
{
	mat[3][0] = pos.x;
	mat[3][1] = pos.y;
	mat[3][2] = pos.z;
}

glm::vec3 MathUtils::GetPosition(const glm::mat4& mat)
{
	return mat[3];
}

glm::mat3 MathUtils::ExtractRotation(const glm::mat4& mat)
{
	glm::mat3 rotation;
	glm::vec3 xAxis = glm::normalize(mat[0]);
	glm::vec3 yAxis = glm::normalize(mat[1]);
	glm::vec3 zAxis = glm::normalize(mat[2]);
	rotation[0][0] = xAxis.x;
	rotation[0][1] = xAxis.y;
	rotation[0][2] = xAxis.z;
	rotation[1][0] = yAxis.x;
	rotation[1][1] = yAxis.y;
	rotation[1][2] = yAxis.z;
	rotation[2][0] = zAxis.x;
	rotation[2][1] = zAxis.y;
	rotation[2][2] = zAxis.z;
	return rotation;
}

glm::vec3 MathUtils::ExtractScale(const glm::mat4& mat)
{
	float scaleX = glm::length(glm::vec3(mat[0]));
	float scaleY = glm::length(glm::vec3(mat[1]));
	float scaleZ = glm::length(glm::vec3(mat[2]));
	return glm::vec3(scaleX, scaleY, scaleZ);
}

void MathUtils::SetScale(glm::mat4& mat, const glm::vec3& scale)
{
	mat[0][0] = scale.x;
	mat[1][1] = scale.y;
	mat[2][2] = scale.z;
}

void MathUtils::ZeroPosition(glm::mat4& mat)
{
	mat[3][0] = 0.0f;
	mat[3][1] = 0.0f;
	mat[3][2] = 0.0f;
}

glm::vec3 MathUtils::GetForward(const glm::mat4& mat)
{
	return glm::normalize(glm::vec3(mat[2]));
}

glm::vec3 MathUtils::GetForward(const glm::mat3& mat)
{
	return glm::normalize(glm::vec3(mat[2]));
}

glm::mat4 MathUtils::BiasMatrix()
{
	glm::mat4 temp(1);
	//MathUtils::SetScale(temp, glm::vec3(0.5));
	//MathUtils::SetPosition(temp, glm::vec3(0.5));
	temp[0][0] = 0.5;
	temp[1][1] = 0.5;
	temp[2][2] = 0.5;
	temp[3][3] = 1.0;
	temp[3][0] = 0.5;
	temp[3][1] = 0.5;
	temp[3][2] = 0.5;
	return temp;
}

glm::vec3
MathUtils::GetAxis(const glm::mat3& mat, int axis)
{
	return glm::vec3(mat[axis]);
}

glm::mat3
MathUtils::CuboidInertiaTensor(const glm::vec3& dimensions)
{
	glm::mat3 inertia(1);
	inertia[0][0] = (1.0f / 12.0f) * (dimensions.y * dimensions.y + dimensions.z * dimensions.z);
	inertia[1][1] = (1.0f / 12.0f) * (dimensions.x * dimensions.x + dimensions.z * dimensions.z);
	inertia[2][2] = (1.0f / 12.0f) * (dimensions.x * dimensions.x + dimensions.y * dimensions.y);

	return inertia;
}

double MathUtils::PI = 3.14159265359;