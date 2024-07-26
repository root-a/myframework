#pragma once
#include <unordered_map>
#include <vector>
#include "Vector3.h"
#include "Vector3F.h"
#include "PoolParty.h"
#include <string>
#include "GraphicsStorage.h"
#include "Node.h"
#include "Frustum.h"

class Object;
class DirectionalLight;
class SpotLight;
class PointLight;
class InstanceSystem;
class FastInstanceSystem;
class Node;
class Material;

class SceneGraph
{
public:
	enum class axis
	{
		x,
		y,
		z
	};
	
    static SceneGraph* Instance();
	Object* addSimpleObject();
	Object* addObject(Object* object);
	Object* addChild();
    Object* addChildTo(Node* parentNodeGR); //minimal function to register object for updates of scenegraph and generates unique id, best for custom objects
	void registerForPicking(Object* object); //helper function for registering objects for picking
	void unregisterForPicking(Object* object);
	std::unordered_map<unsigned int, Object*> pickingList; //picking list, only for look-ups
	std::vector<Object*> renderList; //render list
	std::vector<Object*> pointLights; //render list
	std::vector<Object*> spotLights; //render list
	std::vector<Object*> directionalLights; //render list
	std::vector<Object*> allObjects;
	std::vector<Node*> dirtyDynamicNodes;
	std::vector<Node*> dirtyStaticNodes;
    //Object* SceneObject; //scenegraph root
	Node SceneRoot;
	std::vector<Object*> objectsInFrustum;
	//std::vector<Node*> rootNodes;
	Frustum frustum;
	int count = 0;
	std::vector<Node*> dynamicNodeArray;
	void BuildDynamicNodeArray();
	void SearchNodeForMovables(Node* nodeToSearch);
	void SwitchObjectMovableMode(Object* object, bool movable);
	void SwitchNodeMovableMode(Node* node, bool movable);
	void UpdateDynamicAndStaticArrays(Node* node, bool movable, bool stateChanged);
	int FindNodeIndexInDynamicArray(Node* node);
	void CreateSceneObject();
	void addRandomObject(const glm::vec3& pos = glm::vec3()); //adds random object to the scene
	Object* addInstanceSystem(const char* name = "cube", int count = 3000, const glm::vec3& pos = glm::vec3());
	Object* addInstanceSystemTo(Node* parent, const char* name = "cube", int count = 3000, const glm::vec3& pos = glm::vec3());
	Object* addFastInstanceSystem(const char* name = "cube", int count = 3000, const glm::vec3& pos = glm::vec3());
	Object* addFastInstanceSystemTo(Node* parent, const char* name = "cube", int count = 3000, const glm::vec3& pos = glm::vec3());
	Object* addObject(const char* name = "cube", const glm::vec3& pos = glm::vec3());
	Object* addObjectTo(Node* parent, const char* name = "cube", const glm::vec3& pos = glm::vec3());
	Object* addPhysicObject(const char* name = "cube", const glm::vec3& pos = glm::vec3());
	void addRandomObjects(int num, int min = -20, int max = 20);
	void addRandomlyObjects(const char* name, int num, int min = -20, int max = 20);
	Object* addPointLight(bool castShadow = false, const glm::vec3& position = glm::vec3(), const glm::vec3& color = glm::vec3(1.f, 1.f, 1.f));
	Object* addPointLightTo(Node* parent, bool castShadow = false, const glm::vec3& position = glm::vec3(), const glm::vec3& color = glm::vec3(1.f, 1.f, 1.f));
	Object* addSpotLight(bool castShadow = false, const glm::vec3& position = glm::vec3(), const glm::vec3& color = glm::vec3(1.f, 1.f, 1.f));
	Object* addSpotLightTo(Node* parent, bool castShadow = false, const glm::vec3& position = glm::vec3(), const glm::vec3& color = glm::vec3(1.f, 1.f, 1.f));
	void addRandomPointLight(int min = -20, int max = 20);
	Object* addDirectionalLight(bool castShadow = false, const glm::vec3& color = glm::vec3(1.f, 1.f, 1.f));
	Object* addDirectionalLightTo(Node* parent, bool castShadow = false, const glm::vec3& color = glm::vec3(1.f, 1.f, 1.f));
	void addRandomlyPointLights(int num, int min = -20, int max = 20);
	void addRandomlyPhysicObjects(const char* name, int num, int min = -20, int max = 20);
	Material* CreateDefaultMaterial();
	glm::vec3 generateRandomIntervallVectorCubic(int min, int max);
	glm::vec3 generateRandomIntervallVectorFlat(int min, int max, axis axis = axis::x, int axisHeight = 0);
	glm::vec3 generateRandomIntervallVectorSpherical(int min, int max);

	void Parent(Node* child, Node* newParent);
	void ParentWithOffset(Node* child, Node* newParent, const glm::vec3& newLocalPos, const glm::quat& newLocalOri, const glm::vec3& newLocalScale);
	void ParentWithOffset(Node* child, Node* newParent, const glm::mat4& newLocalTransform);
	void ParentInPlace(Node* child, Node* newParent);
	void Unparent(Node* child);
	void Unparent(Node* child, Node* newParent);
	void UnparentInPlace(Node* child);
	void UnparentInPlace(Node* child, Node* newParent);

	void FrustumCulling();
	void InitializeSceneTree();
	void Update();
	void Clear();
	double updateTransformsTime;
	double updateComponentsTime;
	double updateDynamicArrayTime;
	void ReInit();
private:
    SceneGraph();
    ~SceneGraph();
	
    //copy
    SceneGraph(const SceneGraph&);
    //assign
    SceneGraph& operator=(const SceneGraph&);
	bool dirtyDynamicArray;
};