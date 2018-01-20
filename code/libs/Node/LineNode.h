#pragma once
#include <vector>
#include "MyMathLib.h"
#include "Node.h"

class LineNode : public Node
{
public:
	LineNode();
	~LineNode();

	void UpdateNodeTransform(const Node& parentNode);
	mwm::Vector3 localPosition;

private:

};

