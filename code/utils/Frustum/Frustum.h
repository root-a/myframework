#pragma once
#include "MyMathLib.h"

class Frustum
{
private:	
	glm::vec4 planes[6];
	glm::vec4 col1;
	glm::vec4 col2;
	glm::vec4 col3;
	glm::vec4 col4;
	struct FrustumPlanes
	{
		glm::vec3 normal[6];
		double length[6];
		double w[6];
	};
public:
	Frustum();
	~Frustum();
	void ExtractPlanes(const glm::mat4& VP);
	bool isBoundingSphereInView(const glm::vec3& position, double radius);
	
	FrustumPlanes fPlanes;
}; 