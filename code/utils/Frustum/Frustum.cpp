//
// Created by marwac-9 on 9/17/15.
//
#include "Frustum.h"
using namespace mwm;

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
	Vector4 col1 = Vector4(VP._matrix[0][0], VP._matrix[1][0], VP._matrix[2][0], VP._matrix[3][0]);
	Vector4 col2 = Vector4(VP._matrix[0][1], VP._matrix[1][1], VP._matrix[2][1], VP._matrix[3][1]);
	Vector4 col3 = Vector4(VP._matrix[0][2], VP._matrix[1][2], VP._matrix[2][2], VP._matrix[3][2]);
	Vector4 col4 = Vector4(VP._matrix[0][3], VP._matrix[1][3], VP._matrix[2][3], VP._matrix[3][3]);

	planes[0] = col4 + col1;
	planes[1] = col4 - col1;
	planes[2] = col4 + col2;
	planes[3] = col4 - col2;
	planes[4] = col4 + col3;
	planes[5] = col4 - col3; 
}

bool FrustumManager::isBoundingSphereInView(Vector3 position, float radius)
{
	for (Vector4& plane : planes) {
		Vector3 vect3 = plane.get_xyz();
		Vector3 normal = vect3.vectNormalize();
		float d = plane.vect[3] / vect3.vectLengt();
		if (position.dotAKAscalar(normal) + d + radius <= 0) {
			return false;
		}
	}

	return true;
}
