#pragma once
#include "MyMathLib.h"
#include "PoolParty.h"

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

	PoolParty<Vertex,1000> vertexPool;
	PoolParty<Edge,1000> edgePool;
	PoolParty<Face,1000> facePool;
	std::vector<Vertex*> vertices;
	std::vector<Edge*> edges;
	std::vector<Face*> faces;
private:
	bool checkIfSameVect(Vector3 &vect1, Vector3 &vect2);
	void SplitHalfEdges();
	void CalculateOldPosition();
	void CalculateMidpointPosition();
	void UpdateVertexPositions();
	void UpdateConnections();
};

