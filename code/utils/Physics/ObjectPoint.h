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
			return body->object->bounds->obb.mm.min[axis];
		}
		else
		{
			return body->object->bounds->obb.mm.max[axis];
		}
	}
};