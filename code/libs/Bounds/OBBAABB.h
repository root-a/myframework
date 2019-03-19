#pragma once
#include "MyMathLib.h"
#include "MinMax.h"

namespace mwm
{
struct OBB
{
	mwm::Vector3 extents;
	mwm::Vector3 halfExtents;
	mwm::Matrix3 rot;
	mwm::Matrix4 model;
	mwm::MinMax mm;
	mwm::Vector3F color;
};

struct AABB
{
	mwm::Matrix4 model = Matrix4(1);
	mwm::Vector3F color;
	mwm::Vector3 extents;
};
}