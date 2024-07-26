#pragma once
#include "MyMathLib.h"
#include "PoolParty.h"
#include <string>

class Vertex;
class Edge;
class Face;
//Implementation of half edge mesh for 2D, it generates grid based mesh where cells are made of squares, memory efficient
class HalfEdgeMesh2DSquared
{
public:
	HalfEdgeMesh2DSquared();
	~HalfEdgeMesh2DSquared();
	void Construct(std::string& map, const int width, const int height);
	void ConstructFromFile(const char * path);

	PoolParty<Vertex, 1000> vertexPool;
	PoolParty<Edge, 1000> edgePool;
	PoolParty<Face, 1000> facePool;

	std::vector<Vertex*> vertices;
	std::vector<Edge*> edges;
	std::vector<Face*> faces;
	std::vector<Face*> goals;
	
	std::vector<Vector2> goalsPos;

	Face* startFace;
	Face* endFace;
	Vector2 startFacePos;
	Vector2 endFacePos;

	Face * findNode(const Vector2& position);
	bool isPointInNode(const Vector2& point, Face* node);
	void quadrangulate();
	void optimizeMesh();

private:

};

