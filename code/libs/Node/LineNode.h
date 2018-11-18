#pragma once
#include <vector>
#include "MyMathLib.h"
#include "Node.h"

class LineNode : public Node
{
public:
	LineNode();
	~LineNode();

	void UpdateNode(const Node& parentNode);
	mwm::Vector3 localPosition;

private:

};

