#pragma once
#include "MyMathLib.h"
class Edge;

class Vertex
{
public:
	mwm::Vector2 pos, newPos;
	Vertex();
	~Vertex();
	void* operator new(size_t i){ return _mm_malloc(i, 16); }
	void operator delete(void* p) { _mm_free(p); }
private:

};

