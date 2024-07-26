//
// Created by marwac-9 on 9/17/15.
//
#include "Frustum.h"


Frustum::Frustum()
{
}

Frustum::~Frustum()
{
}

void Frustum::ExtractPlanes(const glm::mat4& VP)
{
	col1.x = VP[0][0], col1.y = VP[1][0], col1.z = VP[2][0], col1.w = VP[3][0];
	col2.x = VP[0][1], col2.y = VP[1][1], col2.z = VP[2][1], col2.w = VP[3][1];
	col3.x = VP[0][2], col3.y = VP[1][2], col3.z = VP[2][2], col3.w = VP[3][2];
	col4.x = VP[0][3], col4.y = VP[1][3], col4.z = VP[2][3], col4.w = VP[3][3];

	planes[0] = col4 + col1;
	planes[1] = col4 - col1;
	planes[2] = col4 + col2;
	planes[3] = col4 - col2;
	planes[4] = col4 + col3;
	planes[5] = col4 - col3;
	glm::vec4& plane0 = planes[0];
	glm::vec4& plane1 = planes[1];
	glm::vec4& plane2 = planes[2];
	glm::vec4& plane3 = planes[3];
	glm::vec4& plane4 = planes[4];
	glm::vec4& plane5 = planes[5];
	fPlanes.normal[0] = glm::normalize(glm::vec3(plane0.x, plane0.y, plane0.z));
	fPlanes.normal[1] = glm::normalize(glm::vec3(plane1.x, plane1.y, plane1.z));
	fPlanes.normal[2] = glm::normalize(glm::vec3(plane2.x, plane2.y, plane2.z));
	fPlanes.normal[3] = glm::normalize(glm::vec3(plane3.x, plane3.y, plane3.z));
	fPlanes.normal[4] = glm::normalize(glm::vec3(plane4.x, plane4.y, plane4.z));
	fPlanes.normal[5] = glm::normalize(glm::vec3(plane5.x, plane5.y, plane5.z));
	fPlanes.length[0] = glm::length(glm::vec3(plane0.x, plane0.y, plane0.z));
	fPlanes.length[1] = glm::length(glm::vec3(plane1.x, plane1.y, plane1.z));
	fPlanes.length[2] = glm::length(glm::vec3(plane2.x, plane2.y, plane2.z));
	fPlanes.length[3] = glm::length(glm::vec3(plane3.x, plane3.y, plane3.z));
	fPlanes.length[4] = glm::length(glm::vec3(plane4.x, plane4.y, plane4.z));
	fPlanes.length[5] = glm::length(glm::vec3(plane5.x, plane5.y, plane5.z));
	fPlanes.w[0] = plane0.w;
	fPlanes.w[1] = plane1.w;
	fPlanes.w[2] = plane2.w;
	fPlanes.w[3] = plane3.w;
	fPlanes.w[4] = plane4.w;
	fPlanes.w[5] = plane5.w;
}

bool Frustum::isBoundingSphereInView(const glm::vec3& position, double radius)
{
	for (int i = 0; i < 6; i++) {
		double d = fPlanes.w[i] / fPlanes.length[i];
		if (glm::dot(position, fPlanes.normal[i]) + d + radius <= 0.0) {
			return false;
		}
	}
	return true;
}