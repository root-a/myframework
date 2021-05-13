#pragma once
#include "MyMathLib.h"
#include "MinMax.h"

struct OBB
{
	Vector3 extents;
	Vector3 halfExtents;
	Matrix3 rot;
	Matrix4 model;
	MinMax mm;
	Vector3F color;
};

struct AABB
{
	Matrix4 model = Matrix4(1);
	Vector3F color;
	Vector3 extents;
};