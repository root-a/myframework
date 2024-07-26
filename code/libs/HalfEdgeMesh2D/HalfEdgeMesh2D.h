#pragma once
#include <list>
#include "MyMathLib.h"
#include "PoolParty.h"
#include <unordered_map>

class Vertex;
class Edge;
class Face;
//Implementation of half edge mesh for 2D, it generates grid based mesh where cells are made of triangles
class HalfEdgeMesh2D
{
public:
	HalfEdgeMesh2D();
	~HalfEdgeMesh2D();
	void Construct(const char * path);

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

	Face* findNode(const Vector2& position);
	bool isPointInNode(const Vector2& point, Face* node);
	void quadrangulate();
	void optimizeMesh();

private:
	
	
};

