#pragma once
#include <string>
#include <vector>
#include "MyMathLib.h"

class Component;

class Node
{
public:
	static Node worldNode;
	Node();
	~Node();
	Node* parent;
	std::vector<Node*> children;
	void addChild(Node* child);
	void removeChild(Node* child);
	mwm::Matrix4 TopDownTransform;
	mwm::Matrix4 LocalizedTopDown;
	
	mwm::Matrix4 LocalScaleM;
	mwm::Matrix4 LocalPositionM;
	mwm::Matrix4 LocalOrientationM;
	mwm::Vector3 localPosition;
	mwm::Quaternion localOrientation;
	mwm::Vector3 localScale;
	std::string name;
	virtual void UpdateNode(const Node& parentNode);
	
	mwm::Vector3 totalScale;
	
	void SetPosition(const mwm::Vector3& vector);
	void SetScale(const mwm::Vector3& vector);
	void Translate(const mwm::Vector3& vector);
	void SetOrientation(const mwm::Quaternion& q);
	void SetRotation(const mwm::Matrix4& m);
	
	mwm::Vector3& GetLocalPosition();
	mwm::Quaternion& GetLocalOrientation();
	mwm::Vector3& GetLocalScale();

	mwm::Matrix3 GetWorldRotation3();
	mwm::Matrix4 GetWorldRotation();
	mwm::Quaternion GetWorldOrientation();
	
	mwm::Vector3 extractScale();
	mwm::Vector3 getScale();
	
	mwm::Vector3 GetWorldPosition() const;
	
	
	void Parent(Node* newParent);
	void ParentWithOffset(Node* newParent, mwm::Vector3& newLocalPos, mwm::Quaternion& newLocalOri, mwm::Vector3& newLocalScale);
	void ParentWithOffset(Node* newParent, mwm::Matrix4& newLocalTransform);
	void ParentInPlace(Node* newParent);
	void Unparent();
	void UnparentInPlace();
	void UnparentInPlaceToNearestStaticAncestor();
	Node* FindNearestStaticAncestor();
	Node* FindNearestMovableAncestor();
	int IsAncestorOf(Node* node);
	Node* FindDescendant(Node* node);
	
	bool SetMovable(bool isMovable);
	bool GetMovable();
	bool GetTotalMovable();
	bool UpdateMovable(Node* node);
private:
	bool movable;
	bool totalMovable;
};