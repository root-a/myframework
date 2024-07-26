#include <vector>
#include <map>
#include <list>
#include "HalfEdgeMesh2DSquared.h"
#include "Vertex.h"
#include "Edge.h"
#include "Face.h"
#include <fstream>
#include <string>
#include <unordered_map>
#include "Optimization.h"

HalfEdgeMesh2DSquared::HalfEdgeMesh2DSquared()
{
	vertexPool.CreatePoolParty();
	edgePool.CreatePoolParty();
	facePool.CreatePoolParty();
}

HalfEdgeMesh2DSquared::~HalfEdgeMesh2DSquared()
{
}

void HalfEdgeMesh2DSquared::Construct(std::string& map, const int width, const int height)
{
	std::vector<Face*> allFaces;
	allFaces.reserve(height*width);
	Face* emptyFace = facePool.Alloc();

	for (int y = 0; y < height; y++)
	{
		//if (map[y*width] == '/' || map[y*width] == ';' || map[y*width] == '#') continue; /* ignore comment line */
		for (int x = 0; x < width; x++)
		{
			int currentCell = y*width + x;
			if (!map[currentCell])
			{
				allFaces.push_back(emptyFace);
			}
			else
			{
				//connecting
				
				//let's get vertices we wanna work with
				Vertex* vertice1 = vertexPool.Alloc();
				Vertex* vertice2 = vertexPool.Alloc();
				Vertex* vertice3 = vertexPool.Alloc();
				Vertex* vertice4 = vertexPool.Alloc();

				vertice1->pos = Vector2(x, (y + 1));
				vertice2->pos = Vector2(x, y);
				vertice3->pos = Vector2((x + 1), y);
				vertice4->pos = Vector2((x + 1), (y + 1));

				//create new edges
				Edge* leftEdge = edgePool.Alloc();
				Edge* topEdge = edgePool.Alloc();
				Edge* rightEdge = edgePool.Alloc();
				Edge* bottomEdge = edgePool.Alloc();

				//connect vertices to edges
				leftEdge->vertex = vertice1;
				topEdge->vertex = vertice2;
				rightEdge->vertex = vertice3;
				bottomEdge->vertex = vertice4;

				//connect edges with next
				leftEdge->next = topEdge;
				topEdge->next = rightEdge;
				rightEdge->next = bottomEdge;
				bottomEdge->next = leftEdge;

				//connect edges with previous
				leftEdge->prev = bottomEdge;
				topEdge->prev = leftEdge;
				rightEdge->prev = topEdge;
				bottomEdge->prev = rightEdge;

				edges.push_back(leftEdge);
				edges.push_back(topEdge);
				edges.push_back(rightEdge);
				edges.push_back(bottomEdge);
				
				//is not at x boundaries?
				if (x > 0)
				{
					//is cell to the left walkable?
					int cellToTheLeft = y*width + (x - 1);
					if (map[cellToTheLeft])
					{
						//pair with the left one
						Edge* lastFaceRightEdge = faces.back()->edge->next->next;
						lastFaceRightEdge->pair = leftEdge;
						leftEdge->pair = lastFaceRightEdge;
					}
				}
				
				//is not at y boundaries?
				if (y > 0)
				{
					//is cell above walkable?
					int cellAbove = (y - 1)*width + x;
					if (map[cellAbove])
					{
						//pair with the cell above
						Edge* aboveFaceBottomEdge = allFaces[cellAbove]->edge->next->next->next;
						aboveFaceBottomEdge->pair = topEdge;
						topEdge->pair = aboveFaceBottomEdge;
					}
				}

				vertices.push_back(vertice1);
				vertices.push_back(vertice2);
				vertices.push_back(vertice3);
				vertices.push_back(vertice4);

				Face* newFace = facePool.Alloc();
				newFace->edge = rightEdge;

				rightEdge->face = newFace;
				rightEdge->next->face = newFace;
				rightEdge->next->next->face = newFace;
				rightEdge->next->next->next->face = newFace;

				faces.push_back(newFace);
				allFaces.push_back(newFace);
				
				if (map[currentCell] == 'S')
				{
					startFace = newFace;
					//startFacePos = newFace->edge->vertex->pos;
					startFacePos = newFace->getMidPointAverage();
				}
				if (map[currentCell] == 'G')
				{
					goals.push_back(newFace);
					goalsPos.push_back(newFace->getMidPointAverage());
					endFace = newFace;
					//endFacePos = newFace2->edge->vertex->pos;
					endFacePos = newFace->getMidPointAverage();
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

void HalfEdgeMesh2DSquared::ConstructFromFile(const char * path)
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
		width = str.length();
	}

	file1.close();

	Construct(map, width, height);
}

Face* HalfEdgeMesh2DSquared::findNode(const Vector2& point)
{
	return Optimization::findNode(point, faces);
}

bool HalfEdgeMesh2DSquared::isPointInNode(const Vector2& point, Face* node)
{
	return Optimization::isPointInNode(point, node);
}

void HalfEdgeMesh2DSquared::quadrangulate()
{
	Optimization::quadrangulate(faces);
}

void HalfEdgeMesh2DSquared::optimizeMesh()
{
	Optimization::optimizeMesh(faces);
}