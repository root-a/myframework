#include <vector>
#include <map>
#include "HalfEdgeMesh2D.h"
#include "Vertex.h"
#include "Edge.h"
#include "Face.h"
#include <unordered_map>
#include <unordered_set>

using namespace cop4530;
using namespace mwm;

//std specialization
namespace std
{
	template <>
	class hash < Vertex* >
	{
	public:
		size_t operator()(const Vertex* val) const {
			return val->pos.a ^ val->pos.b;
			//return val->contactPoint.squareMag();//works

		}
	};

	template <>
	class equal_to < Vertex* >
	{
	public:
		bool operator()(const Vertex* lhs, const Vertex* rhs) const
		{
			return (lhs->pos == rhs->pos);
		}
	};
}

HalfEdgeMesh2D::HalfEdgeMesh2D()
{
	vertexPool.CreatePoolParty();
	edgePool.CreatePoolParty();
	facePool.CreatePoolParty();
}

HalfEdgeMesh2D::~HalfEdgeMesh2D()
{
}

void HalfEdgeMesh2D::Construct(const char * path)
{
	FILE * file;
	errno_t err = fopen_s(&file, path, "r");
	char line[128];
	float x = 0.0f;
	float y = 0.0f;
	int faceSize = 20;
	//std::unordered_map < Vertex*, Vector<Edge*> > test;
	while (fgets(line, sizeof(line), file)) 
	{
		if (line[0] == '/' || line[0] == ';' || line[0] == '#') continue; /* ignore comment line */
		for (int i = 0; i < sizeof(line); i++)
		{
			if (line[i] == '\n' || line[i] == '\0')
			{
				x = 0;
				y = y + faceSize;
				break;
			}
			if (line[i] != 'X')
			{
				
				//connecting
				
					
				//let's get vertices we wanna work with
				Vertex* vertice1 = vertexPool.PoolPartyAlloc();
				Vertex* vertice2 = vertexPool.PoolPartyAlloc();
				Vertex* vertice3 = vertexPool.PoolPartyAlloc();
				Vertex* vertice4 = vertexPool.PoolPartyAlloc();

				vertice1->pos = Vector2(x, y + faceSize);
				vertice2->pos = Vector2(x, y);
				vertice3->pos = Vector2(x + faceSize, y);
				vertice4->pos = Vector2(x + faceSize, y + faceSize);

				//create new edges
				Edge* newEdge1 = edgePool.PoolPartyAlloc();
				Edge* newEdge2 = edgePool.PoolPartyAlloc();
				Edge* newEdge3 = edgePool.PoolPartyAlloc();

				//connect vertices to edges
				newEdge1->vertex = vertice1;
				newEdge2->vertex = vertice2;
				newEdge3->vertex = vertice3;

				//test[vertice1].push_back(newEdge1);
				//test[vertice2].push_back(newEdge2);
				//test[vertice3].push_back(newEdge3);

				//set pair and mid to nullptr
				newEdge1->pair = nullptr;
				newEdge2->pair = nullptr;
				newEdge3->pair = nullptr;
				newEdge1->midVertex = nullptr;
				newEdge2->midVertex = nullptr;
				newEdge3->midVertex = nullptr;

				//connect edges with next
				newEdge1->next = newEdge2;
				newEdge2->next = newEdge3;
				newEdge3->next = newEdge1;

				//connect edges with previous
				newEdge1->prev = newEdge3;
				newEdge2->prev = newEdge1;
				newEdge3->prev = newEdge2;

				edges.push_back(newEdge1);
				edges.push_back(newEdge2);
				edges.push_back(newEdge3);
				
				Edge* newEdge4 = edgePool.PoolPartyAlloc();
				Edge* newEdge5 = edgePool.PoolPartyAlloc();
				Edge* newEdge6 = edgePool.PoolPartyAlloc();

				//connect vertices to edges
				newEdge4->vertex = vertice3;
				newEdge5->vertex = vertice4;
				newEdge6->vertex = vertice1;

				//test[vertice3].push_back(newEdge4);
				//test[vertice4].push_back(newEdge5);
				//test[vertice1].push_back(newEdge6);

				//connect edges with next
				newEdge4->next = newEdge5;
				newEdge5->next = newEdge6;
				newEdge6->next = newEdge4;

				//connect edges with previous
				newEdge4->prev = newEdge6;
				newEdge5->prev = newEdge4;
				newEdge6->prev = newEdge5;

				edges.push_back(newEdge4);
				edges.push_back(newEdge5);
				edges.push_back(newEdge6);

				vertices.push_back(vertice1);
				vertices.push_back(vertice2);
				vertices.push_back(vertice3);
				vertices.push_back(vertice4);

				Face* newFace = facePool.PoolPartyAlloc();
				Face* newFace2 = facePool.PoolPartyAlloc();
				newFace->edge = newEdge1;
				newFace2->edge = newEdge4;

				newEdge1->face = newFace;
				newEdge1->next->face = newFace;
				newEdge1->next->next->face = newFace;

				newEdge4->face = newFace2;
				newEdge4->next->face = newFace2;
				newEdge4->next->next->face = newFace2;

				faces.push_back(newFace);
				faces.push_back(newFace2);
				
				if (line[i] == 'S')
				{
					startFace = newFace;
					//startFacePos = newFace->edge->vertex->pos;
					startFacePos = newFace->getMidPointAverage();
				}
				if (line[i] == 'G')
				{
					goals.push_back(newFace2);
					goalsPos.push_back(newFace2->getMidPointAverage());
					endFace = newFace2;
					//endFacePos = newFace2->edge->vertex->pos;
					endFacePos = newFace2->getMidPointAverage();
				}

				
			}
			x = x + faceSize;
		}
	}
	fclose(file);
	/*
	for (auto& edgei : edges) //for every edge, let's find a pair
	{
		//edge->next->vertex -> give all edges with same source vertex as edge next
		//from that bunch we want edge with next 
		Vector<Edge*>& edgez = test[edgei->next->vertex];
		for (auto& edgej : edgez)
		{
			if (edgej->next->vertex->pos == edgei->vertex->pos)
			{
				edgei->pair = edgej;
				edgej->pair = edgei;
				break;
			}
		}
	}
	*/
	//find pairs
	for (int i = 0; i < edges.size(); i++)
	{	
		for (int j = 0; j < edges.size(); j++)
		{
			if (edges.at(j)->vertex->pos == edges.at(i)->next->vertex->pos) //we could just compare pointers but we have double vertices, not per triangle but per quad
			{
				if (edges.at(j)->next->vertex->pos == edges.at(i)->vertex->pos)
				{
					edges.at(i)->pair = edges.at(j);
					edges.at(j)->pair = edges.at(i);
					break;
				}
			}
		}
	}
	
	/*
	for (int i = 0; i < edges.size(); i++) //error check, if we connected the edge to itself as pair
	{
		if (edges.at(i)->next == edges.at(i)->pair)
		{
			printf("error %d \n", i);
		}
	}
	*/
}

bool HalfEdgeMesh2D::checkIfSameVect(Vector2 &vect1, Vector2 &vect2)
{
	if (vect1.vect[0] == vect2.vect[0] && vect1.vect[1] == vect2.vect[1]) {
		return true;
	}
	else
	{
		return false;
	}
}

Face* HalfEdgeMesh2D::findNode(Vector2 point)
{
	for (int i = 0; i < faces.size(); i++)
	{
		if (this->isPointInNode(point, faces.at(i)))
		{
			return faces.at(i);
		}
	}
	return NULL;
}

bool HalfEdgeMesh2D::isPointInNode(Vector2 point, Face* node)
{
	Edge* currentEdge = node->edge;
	//Looping through every edge in the current node
	do
	{
		Vector2 vectorOfEdge = (currentEdge->next->vertex->pos - currentEdge->vertex->pos).vectNormalize();
		Vector2 vectorToPoint = point-currentEdge->vertex->pos;
		if (vectorToPoint.vect[0] != 0 && vectorToPoint.vect[1] != 0)
		{
			vectorToPoint = vectorToPoint.vectNormalize();
		}
		//clockwise as halfedgemesh is clockwise
		Vector2 sideVectorToEdge = Vector2(vectorOfEdge.vect[1], -vectorOfEdge.vect[0]).vectNormalize();

		if (sideVectorToEdge.dotAKAscalar(vectorToPoint) > 0)
		{
			return false;
		}

		currentEdge = currentEdge->next;

	} while (currentEdge != node->edge);
	return true;
}

void HalfEdgeMesh2D::quadrangulate()
{
	std::list<Face*> toOptimize;
	Vector<Face*> optimized;
	for (int i = 0; i < faces.size(); i++)
	{
		toOptimize.push_back(faces.at(i));
	}
	
	while (!toOptimize.empty()) 
	{
		Face* face = toOptimize.front();
		toOptimize.pop_front();
		if (!face->joined) 
		{
			if (tryToJoin(face)){
				toOptimize.push_back(face);
			}
			else
				optimized.push_back(face);
		}
	}
	faces = optimized;
}

void HalfEdgeMesh2D::optimizeMesh()
{
	std::list<Face*> toOptimize;
	Vector<Face*> optimized;
	for (int i = 0; i < faces.size(); i++)
	{
		toOptimize.push_back(faces.at(i));
	}

	while (!toOptimize.empty())
	{
		Face* face = toOptimize.front();
		toOptimize.pop_front();

		if (!face->joined)
		{
			if (tryToJoin2(face)){
				toOptimize.push_back(face);
			}
			else
				optimized.push_back(face);
		}
	}

	faces = optimized;
}


bool HalfEdgeMesh2D::tryToJoin(Face* face)
{
	Edge* currentEdge = face->edge;
	do {
		Edge* pair = currentEdge->pair;
		if (pair && turnsRight(currentEdge->prev, pair->next) && turnsRight(pair->prev, currentEdge->next)) {
			pair->face->joined = true;
			moveEdgesToFace(pair, face);

			currentEdge->prev->next = pair->next;
			pair->next->prev = currentEdge->prev;

			pair->prev->next = currentEdge->next;
			currentEdge->next->prev = pair->prev;

			if (currentEdge == face->edge)
				face->edge = currentEdge->prev;
			joinSharedEdges(face);
			return true;
		}
		currentEdge = currentEdge->next;
	} while (currentEdge != face->edge);

	return false;
}

bool HalfEdgeMesh2D::tryToJoin2(Face* face)
{
	Edge* currentEdge = face->edge;
	do {
		Edge* pair = currentEdge->pair;
		if (pair && turnsRightOrParallel(currentEdge->prev, pair->next) && turnsRightOrParallel(pair->prev, currentEdge->next)) {
			pair->face->joined = true;
			moveEdgesToFace(pair, face);

			currentEdge->prev->next = pair->next;
			pair->next->prev = currentEdge->prev;

			pair->prev->next = currentEdge->next;
			currentEdge->next->prev = pair->prev;

			if (currentEdge == face->edge)
				face->edge = currentEdge->prev;
			joinSharedEdges(face);
			return true;
		}
		currentEdge = currentEdge->next;
	} while (currentEdge != face->edge);

	return false;
}

bool HalfEdgeMesh2D::turnsRight(Edge* e1, Edge* e2)
{
	Vector2 vectorOfEdge1 = e1->next->vertex->pos - e1->vertex->pos;
	Vector2 vectorOfEdge2 = e2->next->vertex->pos - e2->vertex->pos;
	
	Vector2 sideVectorToEdge = Vector2(vectorOfEdge1.vect[1], -vectorOfEdge1.vect[0]);
	return sideVectorToEdge.dotAKAscalar(vectorOfEdge2) < 0;
}

bool HalfEdgeMesh2D::turnsRightOrParallel(Edge* e1, Edge* e2)
{
	Vector2 vectorOfEdge1 = e1->next->vertex->pos - e1->vertex->pos;
	Vector2 vectorOfEdge2 = e2->next->vertex->pos - e2->vertex->pos;

	Vector2 sideVectorToEdge = Vector2(vectorOfEdge1.vect[1], -vectorOfEdge1.vect[0]);
	return sideVectorToEdge.dotAKAscalar(vectorOfEdge2) <= 0;
}

void HalfEdgeMesh2D::moveEdgesToFace(Edge* startEdge, Face* destFace)
{
	Edge* currentEdge = startEdge;
	do
	{
		currentEdge->face = destFace;

		currentEdge = currentEdge->next;
	} while (currentEdge != startEdge);
}

void HalfEdgeMesh2D::joinSharedEdges(Face* testedFace)
{
	Edge* currentEdge = testedFace->edge;
	do {
		Edge* pair = currentEdge->next->pair;
		if (pair && currentEdge->pair == pair->next) {
			if (currentEdge->next == testedFace->edge)
				testedFace->edge = currentEdge->next->next;
			if (pair->next == pair->face->edge)
				pair->face->edge = pair;
			currentEdge->next = currentEdge->next->next;
			currentEdge->next->prev = currentEdge;
			pair->next = pair->next->next;
			pair->next->prev = pair;
			currentEdge->pair = pair;
			pair->pair = currentEdge;
		}
		currentEdge = currentEdge->next;
	} while (currentEdge != testedFace->edge);
}