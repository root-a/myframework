#include "Scene.h"
#include "Node.h"
#include "Object.h"
#include "Material.h"
#include "GraphicsStorage.h"
#include "PhysicsManager.h"
#include "OBJ.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "InstanceSystem.h"
#include "FastInstanceSystem.h"
#include <chrono>

using namespace mwm;

Scene::Scene()
{
	Init();
}

Scene::~Scene()
{
	
}

void Scene::Init()
{
	objectPool = new PoolParty<Object, 100>();
	objectPool->CreatePoolParty(100);
	SceneObject = objectPool->PoolPartyAlloc();
	allObjects.push_back(SceneObject);
	dirtyNodes.push_back(SceneObject->node);

}

Scene* Scene::Instance()
{
	static Scene instance;

	return &instance;
}

Object* Scene::addChild()
{
	return addChildTo(SceneObject);
}

Object* Scene::addChildTo(Object* parentObject)
{
	Object* child = objectPool->PoolPartyAlloc();
	parentObject->node->addChild(child->node);
	allObjects.push_back(child);
	dirtyNodes.push_back(child->node);
	return child;
}

void Scene::BuildDynamicNodeArray()
{
	if (SceneObject->node->GetMovable())
	{
		dynamicNodeArray.push_back(SceneObject->node);
	}
	else
		SearchNodeForMovables(SceneObject->node);
}

void Scene::SearchNodeForMovables(Node* nodeToSearch)
{
	for (auto* childNode : nodeToSearch->children)
	{
		if (childNode->GetMovable())
		{
			dynamicNodeArray.push_back(childNode);
		}
		else
			SearchNodeForMovables(childNode);
	}
}

void Scene::SwitchObjectMovableMode(Object* object, bool movable) //this will have to be changed so we won't have to call this function but the function on the node itself
{
	SwitchNodeMovableMode(object->node, movable);
}

void Scene::SwitchNodeMovableMode(Node* node, bool movable)
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

int Scene::FindNodeIndexInDynamicArray(Node * node)
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

void Scene::addRandomObject(const Vector3& pos)
{
	Object* newChild = Scene::addChild();
	pickingList[newChild->ID] = newChild;
	renderList.push_back(newChild);

	int index = rand() % (GraphicsStorage::objs.size());
	float rS = (float)(rand() % 5);

	newChild->node->SetPosition(pos);
	newChild->node->SetScale(Vector3(rS, rS, rS));

	Material* newMaterial = new Material();

	auto it = GraphicsStorage::objs.begin();
	std::advance(it, index); 
	
	newChild->AssignMesh(GraphicsStorage::vaos[it->first]);
	newChild->bounds->SetUp(GraphicsStorage::objs[it->first]->center_of_mesh, GraphicsStorage::objs[it->first]->dimensions, GraphicsStorage::objs[it->first]->name);
	newMaterial->AddTexture(GraphicsStorage::textures.at(0));
	newMaterial->AddTexture(GraphicsStorage::textures.at(1));
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
}

Object* Scene::addInstanceSystem(const char * name, int count, const mwm::Vector3 & pos)
{
	return addInstanceSystemTo(SceneObject, name, count, pos);
}

Object* Scene::addInstanceSystemTo(Object * parent, const char * name, int count, const mwm::Vector3 & pos)
{
	Object* newChild = Scene::addChildTo(parent);
	pickingList[newChild->ID] = newChild;
	renderList.push_back(newChild);

	newChild->node->SetPosition(pos);
	newChild->AssignMesh(GraphicsStorage::vaos[name]);
	newChild->bounds->SetUp(GraphicsStorage::objs[name]->center_of_mesh, GraphicsStorage::objs[name]->dimensions, GraphicsStorage::objs[name]->name);
	Material* newMaterial = new Material();
	newMaterial->AddTexture(GraphicsStorage::textures.at(0));
	newMaterial->AddTexture(GraphicsStorage::textures.at(1));
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	InstanceSystem* iSystem = new InstanceSystem(count, GraphicsStorage::objs[name]);
	instanceSystemComponents.push_back(iSystem);
	newChild->AddComponent(iSystem);
	return newChild;
}

Object* Scene::addFastInstanceSystem(const char * name, int count, const mwm::Vector3 & pos)
{
	return addFastInstanceSystemTo(SceneObject, name, count, pos);
}

Object* Scene::addFastInstanceSystemTo(Object * parent, const char * name, int count, const mwm::Vector3 & pos)
{
	Object* newChild = Scene::addChildTo(parent);
	pickingList[newChild->ID] = newChild;
	renderList.push_back(newChild);

	newChild->node->SetPosition(pos);
	newChild->AssignMesh(GraphicsStorage::vaos[name]);
	newChild->bounds->SetUp(GraphicsStorage::objs[name]->center_of_mesh, GraphicsStorage::objs[name]->dimensions, GraphicsStorage::objs[name]->name);
	Material* newMaterial = new Material();
	newMaterial->AddTexture(GraphicsStorage::textures.at(0));
	newMaterial->AddTexture(GraphicsStorage::textures.at(1));
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	FastInstanceSystem* iSystem = new FastInstanceSystem(count, GraphicsStorage::objs[name]);
	fastInstanceSystemComponents.push_back(iSystem);
	newChild->AddComponent(iSystem);
	return newChild;
}

void Scene::registerForPicking(Object * object)
{
	pickingList[object->ID] = object;
}

void Scene::unregisterForPicking(Object * object)
{
	pickingList.erase(object->ID);
}

Object* Scene::addObject(const char* name, const Vector3& pos)
{
	return addObjectTo(SceneObject, name, pos);
}

Object* Scene::addObjectTo(Object* parent, const char* name /*= "cube"*/, const mwm::Vector3& pos /*= mwm::Vector3()*/)
{
	Object* newChild = Scene::addChildTo(parent);
	pickingList[newChild->ID] = newChild;
	renderList.push_back(newChild);

	newChild->node->SetPosition(pos);
	newChild->AssignMesh(GraphicsStorage::vaos[name]);
	newChild->bounds->SetUp(GraphicsStorage::objs[name]->center_of_mesh, GraphicsStorage::objs[name]->dimensions, GraphicsStorage::objs[name]->name);
	Material* newMaterial = new Material();
	newMaterial->AddTexture(GraphicsStorage::textures.at(0));
	newMaterial->AddTexture(GraphicsStorage::textures.at(1));
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	return newChild;
}

void Scene::addRandomObjects(int num, int min, int max)
{
	for(int i = 0; i < num; i++)
    {
		addRandomObject(generateRandomIntervallVectorCubic(min, max));
    }
}

void Scene::addRandomlyObjects(const char* name, int num, int min, int max)
{
	for (int i = 0; i < num; i++)
	{
		Object* obj = addObject(name, generateRandomIntervallVectorCubic(min, max));
	}
}

Object* Scene::addPhysicObject(const char* name, const Vector3& pos)
{
	Object* object = addObject(name, pos);
	RigidBody* body = new RigidBody();
	object->AddComponent(body);
	PhysicsManager::Instance()->RegisterRigidBody(body);
	return object;
}

void Scene::addRandomlyPhysicObjects(const char* name, int num, int min, int max)
{
	for (int i = 0; i < num; i++)
	{
		addPhysicObject(name, generateRandomIntervallVectorCubic(min, max));
	}
}

void Scene::Clear()
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

Object* Scene::addPointLight(bool castShadow, const Vector3& position, const Vector3F& color)
{
	return addPointLightTo(SceneObject, castShadow, position, color);
}

Object * Scene::addPointLightTo(Object * parent, bool castShadow, const mwm::Vector3 & position, const mwm::Vector3F & color)
{
	Object* newChild = Scene::addChildTo(parent);
	PointLight * pointLightComp = new PointLight();
	if (castShadow) pointLightComp->GenerateShadowMapBuffer(1024, 1024);
	newChild->AddComponent(pointLightComp);
	pointLightComponents.push_back(pointLightComp);

	newChild->node->SetPosition(position);
	Material* newMaterial = new Material();
	newMaterial->SetColor(color);
	newChild->AssignMesh(GraphicsStorage::vaos["sphere"]);
	newChild->bounds->SetUp(GraphicsStorage::objs["sphere"]->center_of_mesh, GraphicsStorage::objs["sphere"]->dimensions, GraphicsStorage::objs["sphere"]->name);
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	newChild->node->SetScale(Vector3(4.0, 4.0, 4.0));
	pointLights.push_back(newChild);
	return newChild;
}

Object * Scene::addSpotLight(bool castShadow, const mwm::Vector3 & position, const mwm::Vector3F & color)
{
	return addSpotLightTo(SceneObject, castShadow, position, color);
}

Object * Scene::addSpotLightTo(Object * parent, bool castShadow, const mwm::Vector3 & position, const mwm::Vector3F & color)
{
	Object* newChild = Scene::addChildTo(parent);
	SpotLight * spotLightComp = new SpotLight();
	if (castShadow)
	{
		spotLightComp->GenerateShadowMapBuffer();
		spotLightComp->GenerateBlurShadowMapBuffer();
	}
	newChild->AddComponent(spotLightComp);
	spotLightComponents.push_back(spotLightComp);

	newChild->node->SetPosition(position);
	Material* newMaterial = new Material();
	newMaterial->SetColor(color);
	newChild->AssignMesh(GraphicsStorage::vaos["cone"]);
	newChild->bounds->SetUp(GraphicsStorage::objs["cone"]->center_of_mesh, GraphicsStorage::objs["cone"]->dimensions, GraphicsStorage::objs["cone"]->name);
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	newChild->node->SetScale(Vector3(4.0, 4.0, 4.0));
	spotLights.push_back(newChild);
	return newChild;
}

Object* Scene::addDirectionalLight(bool castShadow, const Vector3F& color /*= Vector3(1, 1, 1)*/)
{
	return addDirectionalLightTo(SceneObject, castShadow, color);
}

Object * Scene::addDirectionalLightTo(Object * parent, bool castShadow, const mwm::Vector3F & color)
{
	Object* newChild = Scene::addChildTo(parent);
	DirectionalLight* dirLightComp = new DirectionalLight();
	dirLightComp->shadowMapActive = castShadow;
	newChild->AddComponent(dirLightComp);
	directionalLightComponents.push_back(dirLightComp);
	Material* newMaterial = new Material();
	newMaterial->SetColor(color);
	newChild->AssignMesh(GraphicsStorage::vaos["plane"]);
	newChild->bounds->SetUp(GraphicsStorage::objs["plane"]->center_of_mesh, GraphicsStorage::objs["plane"]->dimensions, GraphicsStorage::objs["plane"]->name);
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);

	directionalLights.push_back(newChild);

	return newChild;
}

void Scene::addRandomPointLight(int min, int max)
{
	addPointLight(false, generateRandomIntervallVectorCubic(min, max), generateRandomIntervallVectorCubic(0, 255).toFloat()/255.f);
}

Vector3 Scene::generateRandomIntervallVectorCubic(int min, int max)
{
	int range = max - min + 1;

	int rX = rand() % range + min;
	int rY = rand() % range + min;
	int rZ = rand() % range + min;

	return Vector3((double)rX, (double)rY, (double)rZ);
}

Vector3 Scene::generateRandomIntervallVectorSpherical(int min, int max)
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

void Scene::InitializeSceneTree()
{
	SceneObject->node->UpdateNode(*SceneObject->node);
	BuildDynamicNodeArray();
	dirtyNodes.clear();
}

mwm::Vector3 Scene::generateRandomIntervallVectorFlat(int min, int max, axis axis, int axisHeight)
{
	int range = max - min + 1;

	int r1 = rand() % range + min;
	int r2 = rand() % range + min;

	if (axis == x) return Vector3((double)axisHeight, (double)r1, (double)r2);
	else if (axis == y) return Vector3((double)r1, (double)axisHeight, (double)r2);
	else return Vector3((double)r1, (double)r2, (double)axisHeight);
}

void Scene::addRandomlyPointLights(int num, int min, int max)
{
	for (int i = 0; i < num; i++)
	{
		addRandomPointLight(min, max);
	}
}

void Scene::Update()
{
	Bounds::updateBoundsTime = 0.0;
	Bounds::updateMinMaxTime = 0.0;
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;

	start = std::chrono::high_resolution_clock::now();

	for (auto* node : dirtyDynamicNodes)
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
	for (auto* node : dirtyStaticNodes)
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
	
	for (auto* node : dynamicNodeArray)
	{
		node->UpdateNode(*node->parent);
	}
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	updateTransformsTime = elapsed_seconds.count();
	
	start = std::chrono::high_resolution_clock::now();
	for (auto* node : dirtyNodes)
	{
		node->UpdateNode(*node->parent);
	}
	dirtyNodes.clear();
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	updateDirtyTransformsTime = elapsed_seconds.count();

	start = std::chrono::high_resolution_clock::now();
	
	for (auto* object : allObjects)
	{
		object->UpdateComponents();
	}
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	updateComponentsTime = elapsed_seconds.count();
}