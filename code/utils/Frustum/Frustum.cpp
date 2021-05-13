//
// Created by marwac-9 on 9/17/15.
//
#include "Frustum.h"


FrustumManager::FrustumManager()
{
}

FrustumManager::~FrustumManager()
{
}

FrustumManager* FrustumManager::Instance()
{
	static FrustumManager instance;

	return &instance;
}
void FrustumManager::ExtractPlanes(const Matrix4& VP)
{
	col1.x = VP._matrix[0][0], col1.y = VP._matrix[1][0], col1.z = VP._matrix[2][0], col1.w = VP._matrix[3][0];
	col2.x = VP._matrix[0][1], col2.y = VP._matrix[1][1], col2.z = VP._matrix[2][1], col2.w = VP._matrix[3][1];
	col3.x = VP._matrix[0][2], col3.y = VP._matrix[1][2], col3.z = VP._matrix[2][2], col3.w = VP._matrix[3][2];
	col4.x = VP._matrix[0][3], col4.y = VP._matrix[1][3], col4.z = VP._matrix[2][3], col4.w = VP._matrix[3][3];

	planes[0] = col4 + col1;
	planes[1] = col4 - col1;
	planes[2] = col4 + col2;
	planes[3] = col4 - col2;
	planes[4] = col4 + col3;
	planes[5] = col4 - col3; 
}

bool FrustumManager::isBoundingSphereInView(Vector3 position, double radius)
{
	for (Vector4& plane : planes) {
		plane3D = plane.get_xyz();
		plane3DNormal = plane3D.vectNormalize();
		double d = plane.w / plane3D.vectLengt();
		if (position.dotAKAscalar(plane3DNormal) + d + radius <= 0) {
			return false;
		}
	}

	return true;
}
