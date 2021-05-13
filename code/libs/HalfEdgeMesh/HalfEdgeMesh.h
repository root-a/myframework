#pragma once
#include "MyMathLib.h"
#include "PoolParty.h"
#include "Vector.h"

class Vertex;
class Edge;
class Face;
class OBJ;

class HalfEdgeMesh
{
public:
	HalfEdgeMesh();
	~HalfEdgeMesh();
	void Construct(OBJ &object);
	void Subdivide();
	static void ExportMeshToOBJ(HalfEdgeMesh* mesh, OBJ* newOBJ);

private:
	PoolParty<Vertex,1000> vertexPool;
	PoolParty<Edge,1000> edgePool;
	PoolParty<Face,1000> facePool;
	cop4530::Vector<Vertex*> vertices;
	cop4530::Vector<Edge*> edges;
	cop4530::Vector<Face*> faces;
	bool checkIfSameVect(Vector3 &vect1, Vector3 &vect2);
	void SplitHalfEdges();
	void CalculateOldPosition();
	void CalculateMidpointPosition();
	void UpdateVertexPositions();
	void UpdateConnections();
};

