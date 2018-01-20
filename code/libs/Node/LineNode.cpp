#include "LineNode.h"

using namespace mwm;
LineNode::LineNode()
{
	this->TopDownTransform = Matrix4::identityMatrix();
	this->position = Vector3(0.f,0.f,0.f);
}

LineNode::~LineNode()
{
}

void LineNode::UpdateNodeTransform(const Node& parentNode)
{
	this->position = this->localPosition + parentNode.position;
	for (size_t i = 0; i < children.size(); i++)
	{
		children.at(i)->UpdateNodeTransform(*this);
	}
}