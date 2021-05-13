#include "Node.h"
#include "Component.h"

Node::Node()
{
	TopDownTransform.setIdentity();
	LocalScaleM.setIdentity();
	LocalPositionM.setIdentity();
	LocalPositionM.setIdentity();
	LocalOrientationM.setIdentity();
	totalScale.one();
	localScale.one();
	parent = &worldNode;
	movable = false;
	totalMovable = movable;
}

Node::~Node()
{
}

void Node::UpdateNode(const Node& parentNode)
{
	totalScale = localScale * parentNode.totalScale;
	TopDownTransform = LocalScaleM * LocalOrientationM * LocalPositionM * parentNode.TopDownTransform;
	
	for (auto& childNode : children)
	{
		childNode->UpdateNode(*this);
	}
}

void Node::addChild(Node* child)
{
	child->parent = this;
	children.push_back(child);
}

void Node::removeChild(Node * child)
{
	for (size_t i = 0; i < children.size(); i++)
	{
		if (children[i] == child)
		{
			children[i] = children.back();
			children.pop_back();
			return;
		}
	}
}

Vector3 Node::extractScale()
{
	return TopDownTransform.extractScale();
}

Vector3 Node::getScale()
{
	return totalScale;
}

Vector3& Node::GetLocalScale()
{
	return localScale;
}

void Node::SetPosition(const Vector3& vector)
{
	localPosition = vector;
	LocalPositionM.setPosition(vector);
}

Vector3 Node::GetWorldPosition() const
{
	return TopDownTransform.getPosition();
}

Vector3& Node::GetLocalPosition()
{
	return localPosition;
}

void Node::Parent(Node * newParent)
{
	if (parent != newParent)
	{
		if (IsAncestorOf(newParent) != 0)
		{
			newParent->parent->removeChild(newParent);
			parent->addChild(newParent);
			Matrix4& NewParentLocalMatrix = Matrix4::CalculateRelativeTransform(newParent->parent->TopDownTransform, newParent->TopDownTransform);
			newParent->LocalPositionM.setPosition(NewParentLocalMatrix.getPosition());
			newParent->LocalOrientationM = NewParentLocalMatrix.extractRotation();
			newParent->LocalScaleM.setScale(NewParentLocalMatrix.extractScale());
		}
		LocalPositionM.zeroPosition();
		LocalOrientationM.resetRotation();
		LocalScaleM.resetScale();
		parent->removeChild(this);
		newParent->addChild(this);

		UpdateNode(*parent);
		if (totalMovable != newParent->totalMovable) UpdateMovable(this);
	}
}

void Node::ParentWithOffset(Node* newParent, Vector3& newLocalPos, Quaternion& newLocalOri, Vector3& newLocalScale)
{
	if (parent != newParent)
	{
		if (IsAncestorOf(newParent) != 0)
		{
			newParent->parent->removeChild(newParent);
			parent->addChild(newParent);
			Matrix4& NewParentLocalMatrix = Matrix4::CalculateRelativeTransform(newParent->parent->TopDownTransform, newParent->TopDownTransform);
			newParent->LocalPositionM.setPosition(NewParentLocalMatrix.getPosition());
			newParent->LocalOrientationM = NewParentLocalMatrix.extractRotation();
			newParent->LocalScaleM.setScale(NewParentLocalMatrix.extractScale());
		}
		LocalPositionM.setPosition(newLocalPos);
		LocalOrientationM = newLocalOri.ConvertToMatrix();
		LocalScaleM.setScale(newLocalScale);
		parent->removeChild(this);
		newParent->addChild(this);

		UpdateNode(*parent);
		if (totalMovable != newParent->totalMovable) UpdateMovable(this);
	}
}

void Node::ParentWithOffset(Node * newParent, Matrix4& newLocalTransform)
{
	if (parent != newParent)
	{
		if (IsAncestorOf(newParent) != 0)
		{
			newParent->parent->removeChild(newParent);
			parent->addChild(newParent);
			Matrix4& NewParentLocalMatrix = Matrix4::CalculateRelativeTransform(newParent->parent->TopDownTransform, newParent->TopDownTransform);
			newParent->LocalPositionM.setPosition(NewParentLocalMatrix.getPosition());
			newParent->LocalOrientationM = NewParentLocalMatrix.extractRotation();
			newParent->LocalScaleM.setScale(NewParentLocalMatrix.extractScale());
		}
		LocalPositionM.setPosition(newLocalTransform.getPosition());
		LocalOrientationM = newLocalTransform.extractRotation();
		LocalScaleM.setScale(newLocalTransform.extractScale());
		parent->removeChild(this);
		newParent->addChild(this);

		UpdateNode(*parent);
		if (totalMovable != newParent->totalMovable) UpdateMovable(this);
	}
}

void Node::ParentInPlace(Node * newParent)
{
	if (parent != newParent)
	{
		if (IsAncestorOf(newParent) != 0)
		{
			newParent->parent->removeChild(newParent);
			parent->addChild(newParent);
			Matrix4& NewParentLocalMatrix = Matrix4::CalculateRelativeTransform(newParent->parent->TopDownTransform, newParent->TopDownTransform);
			newParent->LocalPositionM.setPosition(NewParentLocalMatrix.getPosition());
			newParent->LocalOrientationM = NewParentLocalMatrix.extractRotation();
			newParent->LocalScaleM.setScale(NewParentLocalMatrix.extractScale());
		}

		Matrix4& NewLocalMatrix = Matrix4::CalculateRelativeTransform(newParent->TopDownTransform, TopDownTransform);
		LocalPositionM.setPosition(NewLocalMatrix.getPosition());
		LocalOrientationM = NewLocalMatrix.extractRotation();
		LocalScaleM.setScale(NewLocalMatrix.extractScale());
		parent->removeChild(this);
		newParent->addChild(this);
		
		
		if (totalMovable != newParent->totalMovable) UpdateMovable(this);
	}
}

void Node::Unparent()
{
	if (parent != &worldNode)
		Parent(&worldNode);
}

void Node::UnparentInPlace()
{
	if (parent != &worldNode)
		ParentInPlace(&worldNode);
}

void Node::UnparentInPlaceToNearestStaticAncestor()
{
	Node* ancestor = FindNearestStaticAncestor();
	if (ancestor != nullptr)
		ParentInPlace(ancestor);
}

Node * Node::FindNearestStaticAncestor()
{
	Node* ancestor = this;
	Node* nearestStaticNode = nullptr;
	do
	{
		ancestor = ancestor->parent;
		if (!ancestor->movable)
		{
			if (nearestStaticNode == nullptr)
			{
				nearestStaticNode = ancestor;
			}
		}
		else
		{
			nearestStaticNode = nullptr;
		}
		
	} while (ancestor != &worldNode);

	return nullptr;
}

Node * Node::FindNearestMovableAncestor()
{
	Node* ancestor = this;

	do
	{
		ancestor = ancestor->parent;
		if (ancestor->movable)
			return ancestor;
	} while (ancestor != &worldNode);
	
	return nullptr;
}

int Node::IsAncestorOf(Node * node)
{
	Node* ancestor = node->parent; //direct parent
	if (ancestor == this)
		return 1;
	
	do
	{
		ancestor = ancestor->parent; //indirect ancestor
		if (ancestor == this)
			return 2;
	} while (ancestor != &worldNode);
	
	return 0; //is not
}

Node * Node::FindDescendant(Node * node)
{
	return nullptr;
}

bool Node::SetMovable(bool isMovable)
{
	movable = isMovable;
	return UpdateMovable(this);
}

bool Node::GetMovable()
{
	return movable;
}

bool Node::GetTotalMovable()
{
	return totalMovable;
}

bool Node::UpdateMovable(Node * node)
{
	bool newTotalMovable = node->movable || node->parent->totalMovable;
	if (node->totalMovable != newTotalMovable)
	{
		node->totalMovable = newTotalMovable;
		for (auto* child : children)
		{
			child->UpdateMovable(child);
		}
		return true;
	}
	return false;
}

void Node::SetScale(const Vector3& vector)
{
	localScale = vector;
	LocalScaleM.setScale(vector);
}

void Node::Translate(const Vector3& vector)
{
	localPosition += vector;
	LocalPositionM.setPosition(localPosition);
}

void Node::SetOrientation(const Quaternion& q)
{
	localOrientation = q;
	LocalOrientationM = localOrientation.ConvertToMatrix();
}

void Node::SetRotation(const Matrix4 & m)
{
	LocalOrientationM = m;
}

Quaternion& Node::GetLocalOrientation()
{
	return localOrientation;
}

Matrix3 Node::GetWorldRotation3()
{
	return TopDownTransform.extractRotation3();
}

Matrix4 Node::GetWorldRotation()
{
	return TopDownTransform.extractRotation();
}

Quaternion Node::GetWorldOrientation()
{
	return TopDownTransform.extractRotation3().toQuaternion();
}

Node Node::worldNode;