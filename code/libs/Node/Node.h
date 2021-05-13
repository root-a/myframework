#pragma once
#include <vector>
#include "MyMathLib.h"
#include <string>

class Component;
class Object;

class Node
{
public:
	static Node worldNode;
	Node();
	~Node();
	Node* parent;
	Object* owner;
	std::vector<Node*> children;
	void addChild(Node* child);
	void removeChild(Node* child);
	Matrix4 TopDownTransform;
	
	Matrix4 LocalScaleM;
	Matrix4 LocalPositionM;
	Matrix4 LocalOrientationM;
	Vector3 localPosition;
	Quaternion localOrientation;
	Vector3 localScale;
	std::string name;
	virtual void UpdateNode(const Node& parentNode);
	
	Vector3 totalScale;
	
	void SetPosition(const Vector3& vector);
	void SetScale(const Vector3& vector);
	void Translate(const Vector3& vector);
	void SetOrientation(const Quaternion& q);
	void SetRotation(const Matrix4& m);
	
	Vector3& GetLocalPosition();
	Quaternion& GetLocalOrientation();
	Vector3& GetLocalScale();

	Matrix3 GetWorldRotation3();
	Matrix4 GetWorldRotation();
	Quaternion GetWorldOrientation();
	
	Vector3 extractScale();
	Vector3 getScale();
	
	Vector3 GetWorldPosition() const;
	
	
	void Parent(Node* newParent);
	void ParentWithOffset(Node* newParent, Vector3& newLocalPos, Quaternion& newLocalOri, Vector3& newLocalScale);
	void ParentWithOffset(Node* newParent, Matrix4& newLocalTransform);
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