#include "Optimization.h"
#include "Edge.h"
#include "Face.h"
#include "Vertex.h"

using namespace cop4530;


Face* Optimization::findNode(const Vector2& point, Vector<Face*>& faces)
{
	for (int i = 0; i < faces.size(); i++)
	{
		if (isPointInNode(point, faces.at(i)))
		{
			return faces.at(i);
		}
	}
	return NULL;
}

Face* Optimization::findNode(const Vector2& point, Face* faces, int mapSize)
{
	for (int i = 0; i < mapSize; i++)
	{
		if (isPointInNode(point, &faces[i]))
		{
			return &faces[i];
		}
	}
	return NULL;
}

bool Optimization::isPointInNode(const Vector2& point, Face* node)
{
	Edge* currentEdge = node->edge;
	//Looping through every edge in the current node
	do
	{
		Vector2 vectorOfEdge = (currentEdge->next->vertex->pos - currentEdge->vertex->pos).vectNormalize();
		Vector2 vectorToPoint = point - currentEdge->vertex->pos;
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

void Optimization::quadrangulate(cop4530::Vector<Face*>& faces)
{
	std::list<Face*> toOptimize;

	for (int i = 0; i < faces.size(); i++)
	{
		toOptimize.push_back(faces.at(i));
	}

	unordered_map<Face*, bool> joined;
	for (size_t i = 0; i < faces.size(); i++)
	{
		Face* face = toOptimize.front();
		toOptimize.pop_front();

		if (!joined[face])
		{
			if (tryToJoin(face, joined)) {
				toOptimize.push_back(face);
			}
		}
	}
	Vector<Face*> optimized;
	optimized.reserve(toOptimize.size());
	for (auto optiFace : toOptimize)
	{
		optimized.push_back(optiFace);
	}

	faces = optimized;
}

int Optimization::quadrangulate(Face * faces, int mapSize)
{
	std::list<Face> toOptimize;

	for (int i = 0; i < mapSize; i++)
	{
		toOptimize.push_back(faces[i]);
	}

	unordered_map<Face*, bool> joined;
	for (size_t i = 0; i < mapSize; i++)
	{
		Face face = toOptimize.front();
		toOptimize.pop_front();

		if (!joined[&face])
		{
			if (tryToJoin(&face, joined)) {
				toOptimize.push_back(face);
			}
		}
	}

	delete[] faces;
	int newMapSize = toOptimize.size();
	faces = new Face[newMapSize];

	int i = 0;
	for (auto& optiFace : toOptimize)
	{
		faces[i] = optiFace;
		i++;
	}

	return newMapSize;
}

void Optimization::optimizeMesh(Vector<Face*>& faces)
{
	std::list<Face*> toOptimize;
	Vector<Face*> optimization;

	for (int i = 0; i < faces.size(); i++)
	{
		toOptimize.push_back(faces.at(i));
	}

	unordered_map<Face*, bool> joined;
	while (!toOptimize.empty())
	{
		Face* face = toOptimize.front();
		toOptimize.pop_front();

		if (!joined[face])
		{
			if (tryToJoin(face, joined)) {
				toOptimize.push_back(face);
			}
			else
			{
				optimization.push_back(face);
			}
		}
	}

	Vector<Face*> optimized;
	for (auto optiFace : optimization)
	{
		if (!joined[optiFace])
		{
			optimized.push_back(optiFace);
		}
	}

	faces = optimized;
}

int Optimization::optimizeMesh(Face* faces, int mapSize)
{
	std::list<Face*> toOptimize;
	Vector<Face> optimization;

	for (int i = 0; i < mapSize; i++)
	{
		toOptimize.push_back(&faces[i]);
	}

	unordered_map<Face*, bool> joined;
	while (!toOptimize.empty())
	{
		Face* face = toOptimize.front();
		toOptimize.pop_front();

		if (!joined[face])
		{
			if (tryToJoin(face, joined)) {
				toOptimize.push_back(face);
			}
			else
			{
				optimization.push_back(*face);
			}
		}
	}

	Vector<Face> optimized;
	optimized.reserve(optimization.size());
	int j = 0;
	for (auto& optiFace : optimization)
	{
		if (!joined[&optiFace])
		{
			optimized[j] = optiFace;
			j++;
		}
	}

	delete[] faces;
	int newMapSize = j + 1;
	faces = new Face[optimized.size()];
	for (int i = 0; i < newMapSize; i++)
	{
		faces[i] = optimized[i];
	}
	return newMapSize;
}

bool Optimization::tryToJoin(Face* face, unordered_map<Face*, bool>& joined)
{
	Edge* currentEdge = face->edge;
	do {
		Edge* pair = currentEdge->pair;

		if (pair && turnsRightOrParallel(currentEdge->prev, pair->next) && turnsRightOrParallel(pair->prev, currentEdge->next))
		{

			joined[pair->face] = true;
			moveEdgesToFace(pair, face);

			currentEdge->prev->next = pair->next;
			pair->next->prev = currentEdge->prev;

			pair->prev->next = currentEdge->next;
			currentEdge->next->prev = pair->prev;

			if (currentEdge == face->edge)
				face->edge = currentEdge->prev->next;
			joinSharedEdges(face);

			return true;
		}
		currentEdge = currentEdge->next;
	} while (currentEdge != face->edge);

	return false;
}

bool Optimization::turnsRightOrParallel(Edge* e1, Edge* e2)
{
	Vector2 vectorOfEdge1 = e1->next->vertex->pos - e1->vertex->pos;
	Vector2 vectorOfEdge2 = e2->next->vertex->pos - e2->vertex->pos;

	Vector2 sideVectorToEdge = Vector2(vectorOfEdge1.y, -vectorOfEdge1.x);

	return sideVectorToEdge.dotAKAscalar(vectorOfEdge2) <= 0;
}

void Optimization::moveEdgesToFace(Edge* startEdge, Face* destFace)
{
	Edge* currentEdge = startEdge;
	do
	{
		currentEdge->face = destFace;
		currentEdge = currentEdge->next;

	} while (currentEdge != startEdge);
}

void Optimization::joinSharedEdges(Face* testedFace)
{
	Edge* currentEdge = testedFace->edge;
	do {
		Edge* pair = currentEdge->next->pair;
		if (pair && currentEdge->pair == pair->next)
		{
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