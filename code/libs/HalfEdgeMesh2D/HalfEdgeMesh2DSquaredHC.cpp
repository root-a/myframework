#include "HalfEdgeMesh2DSquaredHC.h"
#include "EdgeHC.h"
#include "FaceHC.h"
#include <stdio.h>

HalfEdgeMesh2DSquaredHC::HalfEdgeMesh2DSquaredHC()
{
}

HalfEdgeMesh2DSquaredHC::~HalfEdgeMesh2DSquaredHC()
{
	delete[] faces;
}

void HalfEdgeMesh2DSquaredHC::Construct(const unsigned char* map, const int width, const int height)
{
	faces = new FaceHC[height*width];

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int currentCell = y*width + x;
			
			if (map[currentCell])
			{
				FaceHC* newFace = &faces[currentCell];
				newFace->id = currentCell;

				//is not at x boundaries?
				if (x > 0)
				{
					//is cell to the left walkable?
					int cellToTheLeft = y*width + (x - 1);
					if (map[cellToTheLeft])
					{
						//pair with the left one
						EdgeHC* lastFaceEdge = &faces[cellToTheLeft].right;
						lastFaceEdge->pair = &newFace->left;
						newFace->left.pair = lastFaceEdge;
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
						EdgeHC* aboveFaceEdge = &faces[cellAbove].bottom;
						aboveFaceEdge->pair = &newFace->top;
						newFace->top.pair = aboveFaceEdge;
					}
				}
			}
		}
	}
}
