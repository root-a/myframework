#pragma once
#include <vector>
#include "MyMathLib.h"
#include "Node.h"

class LineNode : public Node
{
public:
	LineNode();
	~LineNode();

	void UpdateNode(const LineNode& parentNode);
	glm::vec3 localPosition;

private:

};

