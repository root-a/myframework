#pragma once
#include "MyMathLib.h"
#include "Vector.h"
#include "PoolParty.h"

class Vertex;
class Edge;
class Face;
//Implementation of half edge mesh for 2D, it generates grid based mesh where cells are made of squares, memory efficient
class HalfEdgeMesh2DSquared
{
public:
	HalfEdgeMesh2DSquared();
	~HalfEdgeMesh2DSquared();
	void Construct(std::string map, const int width, const int height);
	void ConstructFromFile(const char * path);

	PoolParty<Vertex, 1000> vertexPool;
	PoolParty<Edge, 1000> edgePool;
	PoolParty<Face, 1000> facePool;

	cop4530::Vector<Vertex*> vertices;
	cop4530::Vector<Edge*> edges;
	cop4530::Vector<Face*> faces;
	cop4530::Vector<Face*> goals;
	
	cop4530::Vector<Vector2> goalsPos;

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

