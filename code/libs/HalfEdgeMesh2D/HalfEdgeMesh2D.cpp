#include <vector>
#include <map>
#include "HalfEdgeMesh2D.h"
#include "Vertex.h"
#include "Edge.h"
#include "Face.h"
#include <fstream>
#include <string>
#include "Optimization.h"

using namespace cop4530;


//std specialization
namespace std
{
	template <>
	class hash < Vertex* >
	{
	public:
		size_t operator()(const Vertex* val) const {
			return val->pos.a ^ val->pos.b;
			//return val->contactPoint.squareLength();//works

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

	std::ifstream file1(path);
	std::string str;
	std::string map = "";
	int width = 0;
	int height = 0;
	//loading the map into a single buffer
	while (std::getline(file1, str))
	{
		map += str;
		height++;
	}
	width = (int)str.length();
	Vector<Face*> allFaces;
	allFaces.reserve(height*width);
	Face* emptyFace = facePool.Alloc();


	file1.close();
	int faceID = 0;
	for (int y = 0; y < height; y++)
	{
		if (map[y*width] == '/' || map[y*width] == ';' || map[y*width] == '#') continue; /* ignore comment line */
		for (int x = 0; x < width; x++)
		{
			int currentCell = y*width + x;
			if (map[currentCell] == 'X')
			{
				allFaces.push_back(emptyFace);
			}
			else
			{
				Face* leftTriangle = facePool.Alloc();
				leftTriangle->id = faceID;
				faceID++;
				Face* rightTriangle = facePool.Alloc();
				rightTriangle->id = faceID;
				faceID++;
				//connecting
					
				//let's get vertices we wanna work with
				Vertex* vertice1 = vertexPool.Alloc();
				Vertex* vertice2 = vertexPool.Alloc();
				Vertex* vertice3 = vertexPool.Alloc();
				Vertex* vertice4 = vertexPool.Alloc();

				vertice1->pos = Vector2((float)x, (float)(y + 1));
				vertice2->pos = Vector2((float)x, (float)y);
				vertice3->pos = Vector2((float)(x + 1), (float)y);
				vertice4->pos = Vector2((float)(x + 1), (float)(y + 1));

				//create new edges
				Edge* leftEdge = edgePool.Alloc();
				Edge* topEdge = edgePool.Alloc();
				Edge* innerRightEdge = edgePool.Alloc();

				//connect vertices to edges
				leftEdge->vertex = vertice1;
				topEdge->vertex = vertice2;
				innerRightEdge->vertex = vertice3;

				//connect edges with next
				leftEdge->next = topEdge;
				topEdge->next = innerRightEdge;
				innerRightEdge->next = leftEdge;

				//connect edges with previous
				leftEdge->prev = innerRightEdge;
				topEdge->prev = leftEdge;
				innerRightEdge->prev = topEdge;

				edges.push_back(leftEdge);
				edges.push_back(topEdge);
				edges.push_back(innerRightEdge);
				
				Edge* rightEdge = edgePool.Alloc();
				Edge* bottomEdge = edgePool.Alloc();
				Edge* innerLeftEdge = edgePool.Alloc();

				//connect vertices to edges
				rightEdge->vertex = vertice3;
				bottomEdge->vertex = vertice4;
				innerLeftEdge->vertex = vertice1;

				//connect edges with next
				rightEdge->next = bottomEdge;
				bottomEdge->next = innerLeftEdge;
				innerLeftEdge->next = rightEdge;

				//connect edges with previous
				rightEdge->prev = innerLeftEdge;
				bottomEdge->prev = rightEdge;
				innerLeftEdge->prev = bottomEdge;

				innerLeftEdge->pair = innerRightEdge;
				innerRightEdge->pair = innerLeftEdge;

				
				//is not at x boundaries?
				if (x > 0)
				{
					//is cell to the right walkable?
					if (map[currentCell-1] != 'X')
					{
						//pair with the last added face (the one to the right)
						Edge* lastFaceRightEdge = faces.back()->edge->next;
						lastFaceRightEdge->pair = leftEdge;
						leftEdge->pair = lastFaceRightEdge;
					}
				}
				
				//is not at y boundaries?
				if (y > 0)
				{
					//is cell above walkable?
					int cellAbove = (y - 1)*width + x;
					if (map[cellAbove] != 'X')
					{
						//pair with the cell above
						Edge* aboveFaceBottomEdge = allFaces[cellAbove]->edge->prev;
						aboveFaceBottomEdge->pair = topEdge;
						topEdge->pair = aboveFaceBottomEdge;
					}
				}
				
				edges.push_back(rightEdge);
				edges.push_back(bottomEdge);
				edges.push_back(innerLeftEdge);

				vertices.push_back(vertice1);
				vertices.push_back(vertice2);
				vertices.push_back(vertice3);
				vertices.push_back(vertice4);

				
				leftTriangle->edge = innerRightEdge;
				rightTriangle->edge = innerLeftEdge;

				leftEdge->face = leftTriangle;
				leftEdge->next->face = leftTriangle;
				leftEdge->next->next->face = leftTriangle;

				rightEdge->face = rightTriangle;
				rightEdge->next->face = rightTriangle;
				rightEdge->next->next->face = rightTriangle;

				faces.push_back(leftTriangle);
				faces.push_back(rightTriangle);
				allFaces.push_back(rightTriangle);
				if (map[currentCell] == 'S')
				{
					startFace = leftTriangle;
					//startFacePos = newFace->edge->vertex->pos;
					startFacePos = leftTriangle->getMidPointAverage();
				}
				if (map[currentCell] == 'G')
				{
					goals.push_back(rightTriangle);
					goalsPos.push_back(rightTriangle->getMidPointAverage());
					endFace = rightTriangle;
					//endFacePos = newFace2->edge->vertex->pos;
					endFacePos = rightTriangle->getMidPointAverage();
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

Face* HalfEdgeMesh2D::findNode(const Vector2& point)
{
	return Optimization::findNode(point, faces);
}

bool HalfEdgeMesh2D::isPointInNode(const Vector2& point, Face* node)
{
	return Optimization::isPointInNode(point, node);
}

void HalfEdgeMesh2D::quadrangulate()
{
	Optimization::quadrangulate(faces);
}

void HalfEdgeMesh2D::optimizeMesh()
{
	Optimization::optimizeMesh(faces);
}