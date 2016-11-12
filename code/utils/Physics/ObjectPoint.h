#pragma once
#include "MyMathLib.h"
#include "RigidBody.h"

struct ObjectPoint
{
	RigidBody* body;
	bool isMin = false;
	ObjectPoint(){}
	ObjectPoint(RigidBody* entRb, bool min){ body = entRb; isMin = min; }
	float value(int axis)
	{
		if (isMin)
		{
			return body->obb.mm.min[axis];
		}
		else
		{
			return body->obb.mm.max[axis];
		}
	}
};