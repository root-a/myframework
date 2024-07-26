#include "Node.h"
#include "Object.h"


Node::Node()
{
	TopDownTransform = glm::mat4(1);
	TopDownTransformF = glm::mat4(1);
	LocalScaleM = glm::mat4(1);
	LocalPositionM = glm::mat4(1);
	LocalOrientationM = glm::mat4(1);
	totalScale = glm::vec3(1);
	localScale = glm::vec3(1);
	localPosition = glm::vec3(1);
	localOrientation = glm::quat(1,0,0,0);
	parent = this;
	movable = false;
	totalMovable = movable;
}

Node::~Node()
{
}

void Node::Init(Object* parent)
{
	Component::Init(parent);

	parent->node = this;
	name = parent->name;
}


void Node::UpdateNode(const Node& parentNode)
{
	totalScale = localScale * parentNode.totalScale;
	TopDownTransform = LocalPositionM * LocalOrientationM * LocalScaleM * parentNode.TopDownTransform;
	TopDownTransformF = TopDownTransform;
	for (auto& childNode : children)
	{
		childNode->UpdateNode(*this);
	}
}

Component* Node::Clone()
{
	return new Node(*this);
}

void Node::addChild(Node* child)
{
	child->parent = this;
	children.push_back(child);
}

void Node::removeChild(Node* child)
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

glm::vec3 Node::extractScale()
{
	return MathUtils::ExtractScale(TopDownTransform);
}

glm::vec3& Node::getScale()
{
	return totalScale;
}

glm::vec3& Node::GetLocalScale()
{
	return localScale;
}

void Node::SetPosition(const glm::vec3& vector)
{
	localPosition = vector;
	MathUtils::SetPosition(LocalPositionM, vector);
}

glm::vec3 Node::GetWorldPosition() const
{
	return MathUtils::GetPosition(TopDownTransform);
}

glm::vec3& Node::GetLocalPosition()
{
	return localPosition;
}

bool Node::Parent(Node* newParent)
{
	if (parent != newParent) //don't parent if current parent is same as new parent
	{
		if (IsAncestorOf(newParent) != 0) //is current node parent of new parent?
		{
			newParent->parent->removeChild(newParent);
			parent->addChild(newParent);
			glm::mat4 NewParentLocalMatrix = MathUtils::CalculateRelativeTransform(newParent->parent->TopDownTransform, newParent->TopDownTransform);
			MathUtils::SetPosition(newParent->LocalPositionM, MathUtils::GetPosition(NewParentLocalMatrix));
			newParent->LocalOrientationM = MathUtils::ExtractRotation(NewParentLocalMatrix);
			MathUtils::SetScale(newParent->LocalScaleM, MathUtils::ExtractScale(NewParentLocalMatrix));
		}
		//probably want a flag for resetting local transform
		//LocalPositionM.zeroPosition();
		//LocalOrientationM.resetRotation();
		//LocalScaleM.resetScale();

		if (parent != nullptr) parent->removeChild(this);
		newParent->addChild(this);

		UpdateNode(*newParent);
		if (totalMovable != newParent->totalMovable)
			return UpdateMovable(this);
		else
			return false;
	}
	return false;
}

bool Node::ParentWithOffset(Node* newParent, const glm::vec3& newLocalPos, const glm::quat& newLocalOri, const glm::vec3& newLocalScale)
{
	if (parent != newParent)
	{
		if (IsAncestorOf(newParent) != 0)
		{
			newParent->parent->removeChild(newParent);
			parent->addChild(newParent);
			glm::mat4 NewParentLocalMatrix = MathUtils::CalculateRelativeTransform(newParent->parent->TopDownTransform, newParent->TopDownTransform);
			MathUtils::SetPosition(newParent->LocalPositionM, MathUtils::GetPosition(NewParentLocalMatrix));
			newParent->LocalOrientationM = MathUtils::ExtractRotation(NewParentLocalMatrix);
			MathUtils::SetScale(newParent->LocalScaleM, MathUtils::ExtractScale(NewParentLocalMatrix));
		}
		MathUtils::SetPosition(LocalPositionM, newLocalPos);
		LocalOrientationM = glm::mat4_cast(newLocalOri);
		MathUtils::SetScale(LocalScaleM, newLocalScale);

		if (parent != nullptr) parent->removeChild(this);
		newParent->addChild(this);

		UpdateNode(*newParent);
		if (totalMovable != newParent->totalMovable)
			return UpdateMovable(this);
		else
			return false;
	}
	return false;
}

bool Node::ParentWithOffset(Node* newParent, const glm::mat4& newLocalTransform)
{
	if (parent != newParent)
	{
		if (IsAncestorOf(newParent) != 0)
		{
			newParent->parent->removeChild(newParent);
			parent->addChild(newParent);
			glm::mat4 NewParentLocalMatrix = MathUtils::CalculateRelativeTransform(newParent->parent->TopDownTransform, newParent->TopDownTransform);
			MathUtils::SetPosition(newParent->LocalPositionM, MathUtils::GetPosition(NewParentLocalMatrix));
			newParent->LocalOrientationM = MathUtils::ExtractRotation(NewParentLocalMatrix);
			MathUtils::SetScale(newParent->LocalScaleM, MathUtils::ExtractScale(NewParentLocalMatrix));
		}
		MathUtils::SetPosition(LocalPositionM, MathUtils::GetPosition(newLocalTransform));
		LocalOrientationM = MathUtils::ExtractRotation(newLocalTransform);
		MathUtils::SetScale(LocalScaleM, MathUtils::ExtractScale(newLocalTransform));
		
		if (parent != nullptr) parent->removeChild(this);
		newParent->addChild(this);

		UpdateNode(*newParent);
		if (totalMovable != newParent->totalMovable)
			return UpdateMovable(this);
		else
			return false;
	}
	return false;
}

bool Node::ParentInPlace(Node* newParent)
{
	if (parent != newParent)
	{
		if (IsAncestorOf(newParent) != 0)
		{
			newParent->parent->removeChild(newParent);
			parent->addChild(newParent);
			glm::mat4 NewParentLocalMatrix = MathUtils::CalculateRelativeTransform(newParent->parent->TopDownTransform, newParent->TopDownTransform);
			MathUtils::SetPosition(newParent->LocalPositionM, MathUtils::GetPosition(NewParentLocalMatrix));
			newParent->LocalOrientationM = MathUtils::ExtractRotation(NewParentLocalMatrix);
			MathUtils::SetScale(newParent->LocalScaleM, MathUtils::ExtractScale(NewParentLocalMatrix));
		}

		//works only with uniform scale
		glm::mat4 NewLocalMatrix = MathUtils::CalculateRelativeTransform(newParent->TopDownTransform, TopDownTransform);
		MathUtils::SetPosition(LocalPositionM, MathUtils::GetPosition(NewLocalMatrix));
		LocalOrientationM = MathUtils::ExtractRotation(NewLocalMatrix);
		MathUtils::SetScale(LocalScaleM, MathUtils::ExtractScale(NewLocalMatrix));

		if (parent != nullptr) parent->removeChild(this);
		newParent->addChild(this);

		if (totalMovable != newParent->totalMovable)
			return UpdateMovable(this);
		else
			return false;
	}
	return false;
}

bool Node::Unparent()
{
	if (parent != nullptr)
	{
		Node* root = FindRootNode();
		return Parent(root);
	}
	return false;
}

bool Node::Unparent(Node* newParent)
{
	if (newParent != nullptr)
	{
		return Parent(newParent);
	}
	else
	{
		if (parent != nullptr)
		{
			parent->removeChild(this);
		}
		parent = newParent;
	}
	return false;
}

bool Node::UnparentInPlace()
{
	if (parent != nullptr)
	{
		Node* root = FindRootNode();
		return ParentInPlace(root);
	}
	return false;
}

bool Node::UnparentInPlace(Node* newParent)
{
	if (newParent != nullptr)
	{
		return ParentInPlace(newParent);
	}
	else
	{
		Node* root = FindRootNode();
		ParentInPlace(root);
		Unparent(newParent);
	}
	return false;
}

bool Node::UnparentInPlaceToNearestStaticAncestor()
{
	Node* ancestor = FindNearestStaticAncestor();
	if (ancestor != nullptr)
		return ParentInPlace(ancestor);
	return false;
}

bool Node::UnparentInPlaceToNearestTotalStaticAncestor()
{
	Node* ancestor = FindNearestTotalStaticAncestor();
	if (ancestor != nullptr)
		return ParentInPlace(ancestor);
	return false;
}

bool Node::UnparentInPlaceToNearestMovableAncestor()
{
	Node* ancestor = FindNearestMovableAncestor();
	if (ancestor != nullptr)
		return ParentInPlace(ancestor);
	return false;
}

Node* Node::FindNearestStaticAncestor()
{
	Node* ancestor = this->parent;
	while (ancestor != ancestor->parent)
	{
		if (!ancestor->movable)
			return ancestor;
		ancestor = ancestor->parent;
	}
	return nullptr;
}

Node* Node::FindNearestTotalStaticAncestor()
{
	Node* ancestor = this->parent;
	//Node* tempStatic = nullptr;
	while (ancestor != ancestor->parent)
	{
		/*
		* using totalMovable
		*/
		/*
		*/
		if (!ancestor->totalMovable)
			return ancestor;
		/*
		* using movable
		if (!ancestor->movable)
		{
			if (tempStatic == nullptr)
			{
				tempStatic = ancestor;
			}
		}
		else
		{
			tempStatic = nullptr;
		}
		*/
		ancestor = ancestor->parent;
	}
	// using totalMovable
	return nullptr;
	// using movable
	//return tempStatic;
}

Node* Node::FindNearestMovableAncestor()
{
	Node* ancestor = this->parent;
	while (ancestor != ancestor->parent)
	{
		if (ancestor->movable)
			return ancestor;
		ancestor = ancestor->parent;
	}
	return nullptr;
}

//is (this) parent(ancestor) of (node)
//am I your parent(ancestor)
//this function can be used to find out if node a is a parent to node b
//it does so by looking if the node b parent(ancestor) is node a recursively all the way to the root
//this means that this function also is good for finding out of node b is a child(descendant) to node a
// 0 no relation
// 1 if it is direct relationship
// 2 if it there are several levels(generations) in differentce 
int Node::IsAncestorOf(Node* node)
{
	Node* ancestor = node->parent; //direct parent
	if (ancestor == this)
		return 1;
	
	while (ancestor != ancestor->parent)
	{
		ancestor = ancestor->parent; //indirect ancestor
		if (ancestor == this)
			return 2;
	}
	return 0; //is not
}

Node* Node::FindRootNode()
{
	Node* currentNode = this;
	while (currentNode->parent != currentNode)
	{
		currentNode = currentNode->parent;
	}
	return currentNode;
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

bool Node::UpdateMovable(Node* node)
{
	bool newTotalMovable = node->movable || (node->parent != nullptr && node->parent->totalMovable);
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

void Node::SetScale(const glm::vec3& vector)
{
	localScale = vector;
	MathUtils::SetScale(LocalScaleM, vector);
}

void Node::Translate(const glm::vec3& vector)
{
	localPosition += vector;
	MathUtils::SetPosition(LocalPositionM, localPosition);
}

void Node::SetOrientation(const glm::quat& q)
{
	localOrientation = glm::normalize(q);//for now
	LocalOrientationM = glm::mat4_cast(localOrientation);
}

void Node::SetRotation(const glm::mat4& m)
{
	LocalOrientationM = m;
	localOrientation = glm::normalize(glm::quat_cast(LocalOrientationM));
}

glm::quat& Node::GetLocalOrientation()
{
	return localOrientation;
}

glm::mat3 Node::GetWorldRotation3()
{
	return MathUtils::ExtractRotation(TopDownTransform);
}

glm::mat4 Node::GetWorldRotation()
{
	return MathUtils::ExtractRotation(TopDownTransform);
}

glm::quat Node::GetWorldOrientation()
{
	return glm::quat_cast(MathUtils::ExtractRotation(TopDownTransform));
}