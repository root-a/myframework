#include "Face.h"
#include "Edge.h"
#include <map>
#include "Vertex.h"



Face::Face()
{
	edge = nullptr;
	previousFace = nullptr;
	id = -1;
}

Face::~Face()
{
}

Vector2 Face::getMidPointAverage()
{
	Edge* currentEdge = this->edge;
	//Looping through every edge in the current node
	int edgecount = 0;
	double x = 0.0;
	double y = 0.0;

	do
	{
		edgecount+=1;

		x += currentEdge->vertex->pos.x;
		y += currentEdge->vertex->pos.y;

		currentEdge = currentEdge->next;

	} while (currentEdge != this->edge);

	x = x / edgecount;
	y = y / edgecount;
	return Vector2(x, y);
}

Vector2 Face::getMidPointMiniMaxi()
{
	Vector2 mini(DBL_MAX, DBL_MAX);
	Vector2 maxi(DBL_MIN, DBL_MIN);

	Edge* currentEdge = this->edge;
	do
	{
		mini = min(currentEdge->vertex->pos, mini);
		maxi = max(currentEdge->vertex->pos, maxi);

		currentEdge = currentEdge->next;

	} while (currentEdge != this->edge);

	return (mini + maxi) * 0.5f;
}

Vector2 Face::min(Vector2& v1, Vector2& v2)
{
	if (v1.vect[0] < v2.vect[0])
	{
		return v1;
	}
	return v2;
}

Vector2 Face::max(Vector2& v1, Vector2& v2)
{
	if (v1.vect[0] > v2.vect[0])
	{
		return v1;
	}
	return v2;
}
