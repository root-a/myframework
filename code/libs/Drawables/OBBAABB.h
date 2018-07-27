#pragma once
#include "MyMathLib.h"
#include "MinMax.h"

namespace mwm
{
struct OBB
{
	mwm::Vector3 halfExtent;//only obb
	mwm::Matrix3 rot;
	mwm::Matrix4 model;
	//prob should include rot matrix instead of model it should not have the scale in it
	mwm::MinMax mm;
	mwm::Vector3F color;
	//mwm::Vector3 centeredPosition;
};

struct AABB
{
	mwm::Matrix4 model;
	mwm::Vector3F color;
};
}