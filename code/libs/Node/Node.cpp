#include "Node.h"

using namespace mwm;
Node::Node()
{
	this->TransformationMatrix = Matrix4::identityMatrix();
	this->TopDownTransform = Matrix4::identityMatrix();
	this->totalScale = Vector3(1.f,1.f,1.f);
	this->localScale = Vector3(1.f,1.f,1.f);
	this->position = Vector3(0.f,0.f,0.f);
	this->orientation = Quaternion(0, Vector3(1.f, 1.f, 1.f));
	this->meshCenter = Vector3();
	this->centeredPosition = Vector3();
}

Node::~Node()
{
}

void Node::UpdateNodeTransform(const Node& parentNode)
{
	totalScale = localScale * parentNode.totalScale;
	this->TransformationMatrix = Matrix4::scale(this->localScale) * orientation.ConvertToMatrix() * Matrix4::translate(this->position);
	this->TopDownTransform = TransformationMatrix*parentNode.TopDownTransform;
	this->CenteredTopDownTransform = Matrix4::translate(meshCenter)*TopDownTransform;
	this->centeredPosition = CenteredTopDownTransform.getPosition();
	for (size_t i = 0; i < children.size(); i++)
	{
		children.at(i)->UpdateNodeTransform(*this);
	}
}

void Node::addChild(Node* child)
{
	this->children.push_back(child);
}