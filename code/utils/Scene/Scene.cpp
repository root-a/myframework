#include "Scene.h"
#include "Node.h"
#include "Object.h"
#include "Material.h"
#include "GraphicsStorage.h"
#include "PhysicsManager.h"
#include "Mesh.h"
#include "OBJ.h"
using namespace mwm;

Scene::Scene()
{
	idCounter = 1;
	SceneObject = new Object();
}

Scene::~Scene()
{
	
}

Scene* Scene::Instance()
{
	static Scene instance;

	return &instance;
}

Object* Scene::addChildTo(Object* parentObject)
{
	Object* child = new Object();
	parentObject->node.addChild(&child->node);

	child->ID = idCounter;
	idCounter++;

	return child;
}

void Scene::addRandomObject(const Vector3& pos)
{
	Object* newChild = Scene::addChildTo(SceneObject);
	pickingList[newChild->ID] = newChild;

	int index = rand() % (GraphicsStorage::meshes.size());
	float rS = (float)(rand() % 5);

	newChild->SetPosition(pos);
	newChild->SetScale(Vector3(rS, rS, rS));

	Material* newMaterial = new Material();

	auto it = GraphicsStorage::meshes.begin();
	std::advance(it, index); 

	newChild->AssignMesh(it->second);
	newMaterial->AssignTexture(GraphicsStorage::textures.at(0));
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
}

Object* Scene::addObjectToScene(const char* name, const Vector3& pos)
{
	Object* newChild = Scene::addChildTo(SceneObject);
	pickingList[newChild->ID] = newChild;

	newChild->SetPosition(pos);
	newChild->AssignMesh(GraphicsStorage::meshes[name]);
	Material* newMaterial = new Material();
	newMaterial->AssignTexture(GraphicsStorage::textures.at(0));
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	return newChild;
}

Object* Scene::addObjectTo(Object* parent, const char* name /*= "cube"*/, const mwm::Vector3& pos /*= mwm::Vector3()*/)
{
	Object* newChild = Scene::addChildTo(parent);
	pickingList[newChild->ID] = newChild;

	newChild->SetPosition(pos);
	newChild->AssignMesh(GraphicsStorage::meshes[name]);
	Material* newMaterial = new Material();
	newMaterial->AssignTexture(GraphicsStorage::textures.at(0));
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
		Object* obj = addObjectToScene(name, generateRandomIntervallVectorCubic(min, max));
	}
}


Object* Scene::addPhysicObject(const char* name, const Vector3& pos)
{
	Object* object = addObjectToScene(name, pos);
	RigidBody* body = new RigidBody(object);
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
	for (auto& obj : pickingList)
	{
		delete obj.second;
	}
	pickingList.clear();

	SceneObject->node.children.clear();
	idCounter = 1;

	for (auto& obj : pointLights)
	{
		delete obj;
	}
	pointLights.clear();

	for (auto& obj : directionalLights)
	{
		delete obj;
	}
	directionalLights.clear();

	SceneObject->ClearComponents(); //just clear components
}

Object* Scene::addPointLight(const Vector3& position, const Vector3F& color)
{
	Object* newChild = Scene::addChildTo(SceneObject);
	
	newChild->SetPosition(position);

	Material* newMaterial = new Material();
	newMaterial->SetColor(color); 
	newChild->AssignMesh(GraphicsStorage::meshes["sphere"]);
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	newChild->SetScale(Vector3(4.0, 4.0, 4.0));
	pointLights.push_back(newChild);
	return newChild;
}

Object* Scene::addDirectionalLight(const Vector3F& direction, const Vector3F& color /*= Vector3(1, 1, 1)*/)
{
	Object* newChild = Scene::addChildTo(SceneObject);

	Material* newMaterial = new Material();
	newMaterial->SetColor(color);
	newChild->AssignMesh(GraphicsStorage::meshes["plane"]);
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);

	directionalLights.push_back(newChild);

	return newChild;
}

void Scene::addRandomPointLight(int min, int max)
{
	addPointLight(generateRandomIntervallVectorCubic(min, max), generateRandomIntervallVectorCubic(0, 255).toFloat()/255.f);
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
	return Vector3(rX / 100.0, rY / 100.0, rZ / 100.0);
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
	SceneObject->node.UpdateNodeTransform(SceneObject->node);
	SceneObject->Update();
	for (auto& obj : pickingList)
	{
		obj.second->Update();
	}
	for (auto& obj : pointLights)
	{
		obj->Update();
	}
	for (auto& obj : directionalLights)
	{
		obj->Update();
	}
}