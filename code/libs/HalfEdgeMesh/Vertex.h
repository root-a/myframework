#pragma once
#include "MyMathLib.h"
class Edge;

class Vertex
{
public:
	Vector3F pos, normal, newPos;
	Vector2F tex;
	Vector4F color;
	Edge * edge;
	Vertex();
	~Vertex();
private:

};

