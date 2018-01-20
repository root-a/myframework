#pragma once
#include "MyMathLib.h"
#include "RigidBody.h"
#include "MinMax.h"

struct ObjectPoint
{
	RigidBody* body;
	bool isMin = false;
	ObjectPoint(){}
	ObjectPoint(RigidBody* entRb, bool min){ body = entRb; isMin = min; }
	double value(int axis)
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