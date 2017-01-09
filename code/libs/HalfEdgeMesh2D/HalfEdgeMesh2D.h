#pragma once
#include <list>
#include "MyMathLib.h"
#include "Vector.h"
#include "PoolParty.h"

class Vertex;
class Edge;
class Face;

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
	Face* findNode(mwm::Vector2 position);
	bool isPointInNode(mwm::Vector2 point, Face* node);
	void quadrangulate();
	void optimizeMesh();
	bool tryToJoin(Face*);
	bool tryToJoin2(Face*);
	void moveEdgesToFace(Edge* edgeOfAnotherFace, Face* currentFace);
	void joinSharedEdges(Face*);
	bool turnsRight(Edge*, Edge*);
	bool turnsRightOrParallel(Edge*, Edge*);
private:
	
	
};

