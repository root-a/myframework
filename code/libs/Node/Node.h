#pragma once
#include <vector>
#include "MyMathLib.h"

class Component;

class Node
{
public:
	Node();
	~Node();
	std::vector<Node*> children;
	void addChild(Node* child);
	mwm::Matrix4 TransformationMatrix;
	mwm::Matrix4 TopDownTransform;
	mwm::Matrix4 CenteredTopDownTransform;
	std::string name;
	virtual void UpdateNode(const Node& parentNode);
	mwm::Vector3 position;
	mwm::Vector3 totalScale;
	mwm::Quaternion orientation;
	mwm::Vector3 meshCenter;
	mwm::Vector3 centeredPosition;
	mwm::Vector3 localScale;
	std::vector<Component*> components;
private:

};

