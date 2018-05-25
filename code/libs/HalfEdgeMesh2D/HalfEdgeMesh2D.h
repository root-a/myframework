#pragma once
#include <list>
#include "MyMathLib.h"
#include "Vector.h"
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

	cop4530::Vector<Vertex*> vertices;
	cop4530::Vector<Edge*> edges;
	cop4530::Vector<Face*> faces;
	cop4530::Vector<Face*> goals;
	
	cop4530::Vector<mwm::Vector2> goalsPos;

	Face* startFace;
	Face* endFace;
	mwm::Vector2 startFacePos;
	mwm::Vector2 endFacePos;

	Face* findNode(const mwm::Vector2& position);
	bool isPointInNode(const mwm::Vector2& point, Face* node);
	void quadrangulate();
	void optimizeMesh();

private:
	
	
};

