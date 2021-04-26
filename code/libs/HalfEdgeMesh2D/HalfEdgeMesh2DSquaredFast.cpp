#include <vector>
#include <map>
#include <list>
#include "HalfEdgeMesh2DSquaredFast.h"
#include "Vertex.h"
#include "Edge.h"
#include "Face.h"
#include <fstream>
#include <string>
#include "Vector.h"
#include <unordered_map>
#include "Optimization.h"

using namespace cop4530;
using namespace mwm;

HalfEdgeMesh2DSquaredFast::HalfEdgeMesh2DSquaredFast()
{
	//emptyFace = new Face();
}

HalfEdgeMesh2DSquaredFast::~HalfEdgeMesh2DSquaredFast()
{
	//delete emptyFace;
	delete[] faces;
	delete[] edges;
	delete[] vertices;
}

void HalfEdgeMesh2DSquaredFast::Construct(std::string& map, const int width, const int height)
{
	mapSize = height*width;
	faces = new Face[mapSize];
	edges = new Edge[mapSize * 4];
	vertices = new Vertex[mapSize * 4];

	for (int y = 0; y < height; y++)
	{
		//if (map[y*width] == '/' || map[y*width] == ';' || map[y*width] == '#') continue; /* ignore comment line */
		for (int x = 0; x < width; x++)
		{
			int currentCell = y*width + x;
			
			if (map[currentCell])
			{
				
				//connecting
				//let's get vertices we wanna work with
				int currentVertex = currentCell * 4;
				Vertex* vertice1 = &vertices[currentVertex];
				Vertex* vertice2 = &vertices[currentVertex + 1];
				Vertex* vertice3 = &vertices[currentVertex + 2];
				Vertex* vertice4 = &vertices[currentVertex + 3];

				vertice1->pos = Vector2(x, (y + 1));
				vertice2->pos = Vector2(x, y);
				vertice3->pos = Vector2((x + 1), y);
				vertice4->pos = Vector2((x + 1), (y + 1));

				//create new edges
				Edge* leftEdge = &edges[currentVertex];
				Edge* topEdge = &edges[currentVertex + 1];
				Edge* rightEdge = &edges[currentVertex + 2];
				Edge* bottomEdge = &edges[currentVertex + 3];

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
				
				//is not at x boundaries?
				if (x > 0)
				{
					//is cell to the left walkable?
					int cellToTheLeft = y*width + (x - 1);
					if (map[cellToTheLeft])
					{
						//pair with the left one
						Edge* lastFaceRightEdge = faces[cellToTheLeft].edge->next->next;
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
						Edge* aboveFaceBottomEdge = faces[cellAbove].edge->next->next->next;
						aboveFaceBottomEdge->pair = topEdge;
						topEdge->pair = aboveFaceBottomEdge;
					}
				}

				Face* newFace = &faces[currentCell];
				newFace->edge = rightEdge;
				newFace->id = currentCell;

				rightEdge->face = newFace;
				rightEdge->next->face = newFace;
				rightEdge->next->next->face = newFace;
				rightEdge->next->next->next->face = newFace;
				
			}
		}
	}
}

void HalfEdgeMesh2DSquaredFast::ConstructFromFile(const char * path)
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

Face* HalfEdgeMesh2DSquaredFast::findNode(const Vector2& point)
{
	return Optimization::findNode(point, faces, mapSize);
}

bool HalfEdgeMesh2DSquaredFast::isPointInNode(const Vector2& point, Face* node)
{
	return Optimization::isPointInNode(point, node);
}

void HalfEdgeMesh2DSquaredFast::quadrangulate()
{
	mapSize = Optimization::quadrangulate(faces, mapSize);
}

void HalfEdgeMesh2DSquaredFast::optimizeMesh()
{
	mapSize = Optimization::optimizeMesh(faces, mapSize);
}