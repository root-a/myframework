#pragma once
#include "MyMathLib.h"

class Edge;

class Face
{
public:
	Edge* edge;
	Face* previousFace;
	Face();
	~Face();
	Vector2 getMidPointAverage();
	Vector2 getMidPointMiniMaxi();
	Vector2 min(Vector2 &v1, Vector2 &v2);
	Vector2 max(Vector2 &v1, Vector2 &v2);
	unsigned int id;
private:
	
};

