#pragma once
#include <unordered_map>
#include "MyMathLib.h"
#include "Vector.h"

class Vertex;
class Edge;
class Face;
//Implementation of half edge mesh for 2D, it generates grid based mesh where cells are made of triangles
class Optimization
{
public:
	Optimization();
	~Optimization();

	static Face* findNode(const Vector2& position, cop4530::Vector<Face*>& faces);
	static Face* findNode(const Vector2& point, Face* faces, int mapSize);

	static bool isPointInNode(const Vector2& point, Face* node);

	static void quadrangulate(cop4530::Vector<Face*>& faces);
	static int quadrangulate(Face* faces, int mapSize);

	static void optimizeMesh(cop4530::Vector<Face*>& faces);
	static int optimizeMesh(Face* faces, int mapSize);
	
private:

	static bool tryToJoin(Face*, std::unordered_map<Face*, bool>&);
	static void moveEdgesToFace(Edge* edgeOfAnotherFace, Face* currentFace);
	static void joinSharedEdges(Face*);
	static bool turnsRightOrParallel(Edge*, Edge*);
};