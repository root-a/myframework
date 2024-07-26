#pragma once
#include "glm/glm.hpp"
class Edge;

class Vertex
{
public:
	glm::vec3 pos, normal, newPos;
	glm::vec2 tex;
	glm::vec4 color;
	Edge * edge;
	Vertex();
	~Vertex();
private:

};

