#pragma once
#include "MyMathLib.h"
#include "MinMax.h"

struct OBB
{
	glm::vec3 extents;
	glm::vec3 halfExtents;
	glm::mat3 rot;
	glm::mat4 model;
	MinMax mm;
	glm::vec3 color;
};

struct AABB
{
	glm::mat4 model = glm::mat4(1);
	glm::vec3 color;
	glm::vec3 extents;
};