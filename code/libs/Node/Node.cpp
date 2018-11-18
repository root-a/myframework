#include "Node.h"
#include "Component.h"
using namespace mwm;
Node::Node()
{
	this->TransformationMatrix = Matrix4(1);
	this->TopDownTransform = Matrix4(1);
	this->totalScale = Vector3(1.0,1.0,1.0);
	this->localScale = Vector3(1.0,1.0,1.0);
	this->position = Vector3(0.0,0.0,0.0);
	this->orientation = Quaternion(0.0, Vector3());
	this->meshCenter = Vector3();
	this->centeredPosition = Vector3();
}

Node::~Node()
{
	for (auto& component : components)
	{
		delete component;
	}
	components.clear();
}

void Node::UpdateNode(const Node& parentNode)
{
	totalScale = localScale * parentNode.totalScale;
	this->TransformationMatrix = Matrix4::scale(this->localScale) * orientation.ConvertToMatrix() * Matrix4::translate(this->position);
	this->TopDownTransform = TransformationMatrix*parentNode.TopDownTransform;
	this->CenteredTopDownTransform = Matrix4::translate(meshCenter)*TopDownTransform;
	this->centeredPosition = CenteredTopDownTransform.getPosition();
	for (auto& component : components)
	{
		component->Update();
	}
	for (auto& childNode : children)
	{
		childNode->UpdateNode(*this);
	}
}

void Node::addChild(Node* child)
{
	this->children.push_back(child);
}