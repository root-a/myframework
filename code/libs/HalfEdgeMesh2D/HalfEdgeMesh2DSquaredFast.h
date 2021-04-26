#pragma once
#include "MyMathLib.h"
#include <string>

class Vertex;
class Edge;
class Face;

//Implementation of half edge mesh for 2D, it generates grid based mesh where cells are made of squares, faster but consumes more memory
class HalfEdgeMesh2DSquaredFast
{
public:
	HalfEdgeMesh2DSquaredFast();
	~HalfEdgeMesh2DSquaredFast();
	void Construct(std::string& map, const int width, const int height);
	void ConstructFromFile(const char * path);

	Vertex* vertices;
	Edge* edges;
	Face* faces;
	int mapSize;

	Face * findNode(const mwm::Vector2& position);
	bool isPointInNode(const mwm::Vector2& point, Face* node);
	void quadrangulate();
	void optimizeMesh();
private:

};

