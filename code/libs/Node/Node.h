#pragma once
#include <vector>
#include "MyMathLib.h"
#include <string>
#include "Component.h"
#include <glm/glm.hpp>
class Object;

class Node : public Component
{
public:
	Node();
	~Node();
	void Init(Object* parent);
	Node* parent;
	std::vector<Node*> children;
	void addChild(Node* child);
	void removeChild(Node* child);
	glm::mat4 TopDownTransform;
	glm::mat4 TopDownTransformF;
	
	glm::mat4 LocalScaleM;
	glm::mat4 LocalPositionM;
	glm::mat4 LocalOrientationM;
	glm::vec3 localPosition;
	glm::quat localOrientation;
	glm::vec3 localScale;
	glm::vec3 totalScale;
	std::string name;
	virtual void UpdateNode(const Node& parentNode);
	Component* Clone();
	
	void SetPosition(const glm::vec3& vector);
	void SetScale(const glm::vec3& vector);
	void Translate(const glm::vec3& vector);
	void SetOrientation(const glm::quat& q);
	void SetRotation(const glm::mat4& m);
	
	glm::vec3& GetLocalPosition();
	glm::quat& GetLocalOrientation();
	glm::vec3& GetLocalScale();

	glm::mat3 GetWorldRotation3();
	glm::mat4 GetWorldRotation();
	glm::quat GetWorldOrientation();
	
	glm::vec3 extractScale();
	glm::vec3& getScale();
	
	glm::vec3 GetWorldPosition() const;
	
	
	bool Parent(Node* newParent);
	bool ParentWithOffset(Node* newParent, const glm::vec3& newLocalPos, const glm::quat& newLocalOri, const glm::vec3& newLocalScale);
	bool ParentWithOffset(Node* newParent, const glm::mat4& newLocalTransform);
	bool ParentInPlace(Node* newParent);
	bool Unparent();
	bool Unparent(Node* newParent);
	bool UnparentInPlace();
	bool UnparentInPlace(Node* newParent);
	bool UnparentInPlaceToNearestStaticAncestor();
	bool UnparentInPlaceToNearestTotalStaticAncestor();
	bool UnparentInPlaceToNearestMovableAncestor();
	Node* FindNearestStaticAncestor();
	Node* FindNearestTotalStaticAncestor();
	Node* FindNearestMovableAncestor();
	int IsAncestorOf(Node* node);
	Node* FindRootNode();
	
	bool SetMovable(bool isMovable);
	bool GetMovable();
	bool GetTotalMovable();
	bool UpdateMovable(Node* node);
private:
	bool movable;
	bool totalMovable;
};