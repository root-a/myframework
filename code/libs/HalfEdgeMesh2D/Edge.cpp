#include "Edge.h"
#include "Vertex.h"



Edge::Edge()
{
	vertex, midVertex = nullptr;
	next = nullptr;
	prev = nullptr;
	pair = nullptr;
	face = nullptr;
}

Edge::~Edge()
{
}

Vector2 Edge::Midpoint()
{
	Vector2 midpoint = (this->vertex->pos + this->next->vertex->pos) / 2.f;
	return midpoint;
}