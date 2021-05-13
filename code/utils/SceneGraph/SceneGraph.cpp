#include "SceneGraph.h"
#include "Node.h"
#include "Object.h"
#include "Material.h"
#include "GraphicsStorage.h"
#include "PhysicsManager.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "InstanceSystem.h"
#include "FastInstanceSystem.h"
#include <chrono>
#include "Frustum.h"



SceneGraph::SceneGraph()
{
	Init();
}

SceneGraph::~SceneGraph()
{
	
}

void SceneGraph::Init()
{
	objectPool = new PoolParty<Object, 100>();
	objectPool->CreatePoolParty(100);
	SceneObject = objectPool->PoolPartyAlloc();
	SceneObject->name = "scene";
	allObjects.push_back(SceneObject);
	dirtyNodes.push_back(SceneObject->node);
}

SceneGraph* SceneGraph::Instance()
{
	static SceneGraph instance;

	return &instance;
}

Object* SceneGraph::addChild()
{
	return addChildTo(SceneObject);
}

Object* SceneGraph::addChildTo(Object* parentObject)
{
	Object* child = objectPool->PoolPartyAlloc();
	parentObject->node->addChild(child->node);
	allObjects.push_back(child);
	dirtyNodes.push_back(child->node);
	return child;
}

void SceneGraph::BuildDynamicNodeArray()
{
	if (SceneObject->node->GetMovable())
	{
		dynamicNodeArray.push_back(SceneObject->node);
	}
	else
		SearchNodeForMovables(SceneObject->node);
}

void SceneGraph::SearchNodeForMovables(Node* nodeToSearch)
{
	for (auto childNode : nodeToSearch->children)
	{
		if (childNode->GetMovable())
		{
			dynamicNodeArray.push_back(childNode);
		}
		else
			SearchNodeForMovables(childNode);
	}
}

void SceneGraph::SwitchObjectMovableMode(Object* object, bool movable) //this will have to be changed so we won't have to call this function but the function on the node itself
{
	SwitchNodeMovableMode(object->node, movable);
}

void SceneGraph::SwitchNodeMovableMode(Node* node, bool movable)
{
	bool stateChanged = node->SetMovable(movable); 
	if (stateChanged)
	{
		if (movable)
			dirtyDynamicNodes.push_back(node);
		else
			dirtyStaticNodes.push_back(node);
	}
	
}

int SceneGraph::FindNodeIndexInDynamicArray(Node * node)
{
	for (size_t i = 0; i < dynamicNodeArray.size(); i++)
	{
		if (dynamicNodeArray[i] == node)
		{
			return i;
		}
	}
	return -1;
}

Material* SceneGraph::CreateDefaultMaterial()
{
	Material* newMaterial = new Material();
	newMaterial->AssignTexture(GraphicsStorage::textures["default_diffuse"], 0);
	newMaterial->AssignTexture(GraphicsStorage::textures["default_normal_dtx5"], 1);
	newMaterial->AssignTexture(GraphicsStorage::textures["default_aometrough"], 2);
	return newMaterial;
}

void SceneGraph::addRandomObject(const Vector3& pos)
{
	Object* newChild = SceneGraph::addChild();
	pickingList[newChild->ID] = newChild;
	renderList.push_back(newChild);

	int index = rand() % (GraphicsStorage::objs.size());
	float rS = (float)(rand() % 5);

	newChild->node->SetPosition(pos);
	newChild->node->SetScale(Vector3(rS, rS, rS));

	auto it = GraphicsStorage::objs.begin();
	std::advance(it, index); 
	
	newChild->vao = GraphicsStorage::vaos[it->first];
	newChild->bounds->SetUp(GraphicsStorage::vaos[it->first]->center, GraphicsStorage::vaos[it->first]->dimensions, GraphicsStorage::vaos[it->first]->name);
	Material* newMaterial = CreateDefaultMaterial();
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
}

Object* SceneGraph::addInstanceSystem(const char * name, int count, const Vector3 & pos)
{
	return addInstanceSystemTo(SceneObject, name, count, pos);
}

Object* SceneGraph::addInstanceSystemTo(Object * parent, const char * name, int count, const Vector3 & pos)
{
	Object* newChild = SceneGraph::addChildTo(parent);
	newChild->name = name;
	pickingList[newChild->ID] = newChild;
	renderList.push_back(newChild);

	newChild->node->SetPosition(pos);
	newChild->vao = GraphicsStorage::vaos[name];
	newChild->bounds->SetUp(GraphicsStorage::vaos[name]->center, GraphicsStorage::vaos[name]->dimensions, GraphicsStorage::vaos[name]->name);
	Material* newMaterial = CreateDefaultMaterial();
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	InstanceSystem* iSystem = new InstanceSystem(count, GraphicsStorage::objs[name]);
	instanceSystemComponents.push_back(iSystem);
	newChild->AddComponent(iSystem);
	return newChild;
}

Object* SceneGraph::addFastInstanceSystem(const char * name, int count, const Vector3 & pos)
{
	return addFastInstanceSystemTo(SceneObject, name, count, pos);
}

Object* SceneGraph::addFastInstanceSystemTo(Object * parent, const char * name, int count, const Vector3 & pos)
{
	Object* newChild = SceneGraph::addChildTo(parent);
	newChild->name = name;
	pickingList[newChild->ID] = newChild;
	renderList.push_back(newChild);

	newChild->node->SetPosition(pos);
	newChild->vao = GraphicsStorage::vaos[name];
	newChild->bounds = new Bounds();
	newChild->bounds->SetUp(GraphicsStorage::vaos[name]->center, GraphicsStorage::vaos[name]->dimensions, GraphicsStorage::vaos[name]->name);
	Material* newMaterial = CreateDefaultMaterial();
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	FastInstanceSystem* iSystem = new FastInstanceSystem(count, GraphicsStorage::objs[name]);
	fastInstanceSystemComponents.push_back(iSystem);
	newChild->AddComponent(iSystem);
	return newChild;
}

void SceneGraph::registerForPicking(Object * object)
{
	pickingList[object->ID] = object;
}

void SceneGraph::unregisterForPicking(Object * object)
{
	pickingList.erase(object->ID);
}

Object* SceneGraph::addObject(const char* name, const Vector3& pos)
{
	return addObjectTo(SceneObject, name, pos);
}

Object* SceneGraph::addObjectTo(Object* parent, const char* name /*= "cube"*/, const Vector3& pos /*= Vector3()*/)
{
	Object* newChild = SceneGraph::addChildTo(parent);
	newChild->name = name;
	pickingList[newChild->ID] = newChild;
	renderList.push_back(newChild);

	newChild->node->SetPosition(pos);
	newChild->vao = GraphicsStorage::vaos[name];
	newChild->bounds = new Bounds();
	newChild->AddComponent(newChild->bounds, true);
	newChild->bounds->SetUp(GraphicsStorage::vaos[name]->center, GraphicsStorage::vaos[name]->dimensions, GraphicsStorage::vaos[name]->name);
	Material* newMaterial = CreateDefaultMaterial();
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	return newChild;
}

void SceneGraph::addRandomObjects(int num, int min, int max)
{
	for(int i = 0; i < num; i++)
    {
		addRandomObject(generateRandomIntervallVectorCubic(min, max));
    }
}

void SceneGraph::addRandomlyObjects(const char* name, int num, int min, int max)
{
	for (int i = 0; i < num; i++)
	{
		Object* obj = addObject(name, generateRandomIntervallVectorCubic(min, max));
	}
}

Object* SceneGraph::addPhysicObject(const char* name, const Vector3& pos)
{
	Object* object = addObject(name, pos);
	RigidBody* body = new RigidBody();
	object->AddComponent(body, true);
	PhysicsManager::Instance()->RegisterRigidBody(body);
	return object;
}

void SceneGraph::addRandomlyPhysicObjects(const char* name, int num, int min, int max)
{
	for (int i = 0; i < num; i++)
	{
		addPhysicObject(name, generateRandomIntervallVectorCubic(min, max));
	}
}

void SceneGraph::Clear()
{
	SceneObject->ResetIDs();
	dynamicNodeArray.clear();
	dirtyDynamicNodes.clear();
	dirtyStaticNodes.clear();
	dirtyNodes.clear();
	allObjects.clear();
	delete objectPool;
	Init();

	renderList.clear();
	pickingList.clear();

	pointLights.clear();
	spotLights.clear();
	directionalLights.clear();

	directionalLightComponents.clear();
	spotLightComponents.clear();
	pointLightComponents.clear();
	instanceSystemComponents.clear();
	fastInstanceSystemComponents.clear();
}

Object* SceneGraph::addPointLight(bool castShadow, const Vector3& position, const Vector3F& color)
{
	return addPointLightTo(SceneObject, castShadow, position, color);
}

Object * SceneGraph::addPointLightTo(Object * parent, bool castShadow, const Vector3 & position, const Vector3F & color)
{
	Object* newChild = SceneGraph::addChildTo(parent);
	PointLight * pointLightComp = new PointLight();
	if (castShadow) pointLightComp->GenerateShadowMapBuffer(1024, 1024);
	newChild->AddComponent(pointLightComp, true);
	pointLightComponents.push_back(pointLightComp);

	newChild->node->SetPosition(position);
	Material* newMaterial = new Material();
	newMaterial->SetColor(color);
	newChild->vao = GraphicsStorage::vaos["sphere"];
	newChild->bounds = new Bounds();
	newChild->AddComponent(newChild->bounds, true);
	newChild->bounds->SetUp(GraphicsStorage::vaos["sphere"]->center, GraphicsStorage::vaos["sphere"]->dimensions, GraphicsStorage::vaos["sphere"]->name);
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	newChild->node->SetScale(Vector3(4.0, 4.0, 4.0));
	pointLights.push_back(newChild);
	return newChild;
}

Object * SceneGraph::addSpotLight(bool castShadow, const Vector3 & position, const Vector3F & color)
{
	return addSpotLightTo(SceneObject, castShadow, position, color);
}

Object * SceneGraph::addSpotLightTo(Object * parent, bool castShadow, const Vector3 & position, const Vector3F & color)
{
	Object* newChild = SceneGraph::addChildTo(parent);
	SpotLight * spotLightComp = new SpotLight();
	if (castShadow)
	{
		spotLightComp->GenerateShadowMapBuffer();
		spotLightComp->GenerateBlurShadowMapBuffer();
	}
	newChild->AddComponent(spotLightComp, true);
	spotLightComponents.push_back(spotLightComp);

	newChild->node->SetPosition(position);
	Material* newMaterial = new Material();
	newMaterial->SetColor(color);
	newChild->vao = GraphicsStorage::vaos["cone"];
	newChild->bounds = new Bounds();
	newChild->AddComponent(newChild->bounds, true);
	newChild->bounds->SetUp(GraphicsStorage::vaos["cone"]->center, GraphicsStorage::vaos["cone"]->dimensions, GraphicsStorage::vaos["cone"]->name);
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	newChild->node->SetScale(Vector3(4.0, 4.0, 4.0));
	spotLights.push_back(newChild);
	return newChild;
}

Object* SceneGraph::addDirectionalLight(bool castShadow, const Vector3F& color /*= Vector3(1, 1, 1)*/)
{
	return addDirectionalLightTo(SceneObject, castShadow, color);
}

Object * SceneGraph::addDirectionalLightTo(Object * parent, bool castShadow, const Vector3F & color)
{
	Object* newChild = SceneGraph::addChildTo(parent);
	DirectionalLight* dirLightComp = new DirectionalLight();
	dirLightComp->shadowMapActive = castShadow;
	newChild->AddComponent(dirLightComp, true);
	directionalLightComponents.push_back(dirLightComp);
	Material* newMaterial = new Material();
	newMaterial->SetColor(color);
	newChild->vao = GraphicsStorage::vaos["plane"];
	//newChild->bounds->SetUp(GraphicsStorage::vaos["plane"]->center, GraphicsStorage::vaos["plane"]->dimensions, GraphicsStorage::vaos["plane"]->name);
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);

	directionalLights.push_back(newChild);

	return newChild;
}

void SceneGraph::addRandomPointLight(int min, int max)
{
	addPointLight(false, generateRandomIntervallVectorCubic(min, max), generateRandomIntervallVectorCubic(0, 255).toFloat()/255.f);
}

Vector3 SceneGraph::generateRandomIntervallVectorCubic(int min, int max)
{
	int range = max - min + 1;

	int rX = rand() % range + min;
	int rY = rand() % range + min;
	int rZ = rand() % range + min;

	return Vector3((double)rX, (double)rY, (double)rZ);
}

Vector3 SceneGraph::generateRandomIntervallVectorSpherical(int min, int max)
{
	int range = max * 2 + 1;
	int rX = 0;
	int rY = 0;
	int rZ = 0;
	int sum = 0;
	do {
		rX = rand() % range - max;
		rY = rand() % range - max;
		rZ = rand() % range - max;
		sum = rX*rX + rY*rY + rZ*rZ;
	} while (sum > max*max || sum < min*min); //inside sphere change to < for outside of sphere
	return Vector3(rX, rY, rZ);
}

void SceneGraph::FrustumCulling()
{
	objectsInFrustum.clear();
	for (auto object : allObjects)
	{
		Bounds* bounds = object->GetComponent<Bounds>(); // we should have list of all bounds components
		if (bounds != nullptr) //checking existing pointer is faster or getting it from the map but whatever!
		{
			object->inFrustum = FrustumManager::Instance()->isBoundingSphereInView(bounds->centeredPosition, bounds->circumRadius);
			if (object->inFrustum) objectsInFrustum.push_back(object);
		}
		else
		{
			object->inFrustum = true;
			objectsInFrustum.push_back(object);
		}
	}
}

void SceneGraph::InitializeSceneTree()
{
	SceneObject->node->UpdateNode(*SceneObject->node);
	BuildDynamicNodeArray();
	dirtyNodes.clear();
}

Vector3 SceneGraph::generateRandomIntervallVectorFlat(int min, int max, axis axis, int axisHeight)
{
	int range = max - min + 1;

	int r1 = rand() % range + min;
	int r2 = rand() % range + min;

	if (axis == x) return Vector3((double)axisHeight, (double)r1, (double)r2);
	else if (axis == y) return Vector3((double)r1, (double)axisHeight, (double)r2);
	else return Vector3((double)r1, (double)r2, (double)axisHeight);
}

void SceneGraph::addRandomlyPointLights(int num, int min, int max)
{
	for (int i = 0; i < num; i++)
	{
		addRandomPointLight(min, max);
	}
}

void SceneGraph::Update()
{
	Bounds::updateBoundsTime = 0.0;
	Bounds::updateMinMaxTime = 0.0;
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;

	start = std::chrono::high_resolution_clock::now();

	for (auto node : dirtyDynamicNodes)
	{
		bool movable = node->GetMovable();
		bool totalMovableParent = node->parent->GetTotalMovable();
		bool totalMovable = node->GetTotalMovable();
		if (movable && !totalMovableParent)
		{
			dynamicNodeArray.push_back(node);
		}
	}
	dirtyDynamicNodes.clear();
	for (auto node : dirtyStaticNodes)
	{
		if (!node->GetTotalMovable() || (!node->GetMovable() && node->parent->GetTotalMovable()))//we remove two types, one is when node total is static second is when node is static but total is dynamic meaning a node above is dynamic
		{
			int dynamicNodeIndex = FindNodeIndexInDynamicArray(node);
			if (dynamicNodeIndex != -1)
			{
				dynamicNodeArray[dynamicNodeIndex] = dynamicNodeArray.back();
				dynamicNodeArray.pop_back();
			}
		}
	}
	dirtyStaticNodes.clear();

	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	updateDynamicArrayTime = elapsed_seconds.count();

	start = std::chrono::high_resolution_clock::now();
	
	for (auto node : dynamicNodeArray)
	{
		node->UpdateNode(*node->parent);
	}
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	updateTransformsTime = elapsed_seconds.count();
	
	start = std::chrono::high_resolution_clock::now();
	for (auto node : dirtyNodes)
	{
		node->UpdateNode(*node->parent);
	}
	dirtyNodes.clear();
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	updateDirtyTransformsTime = elapsed_seconds.count();

	start = std::chrono::high_resolution_clock::now();
	
	for (auto object : allObjects)
	{
		object->Update();
		object->UpdateComponents();
	}
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	updateComponentsTime = elapsed_seconds.count();
}