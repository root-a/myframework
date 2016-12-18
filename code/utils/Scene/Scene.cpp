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
	MainPointLight = new Object();
	MainDirectionalLight = new Object();
}

Scene::~Scene()
{
	
}

Scene* Scene::Instance()
{
	static Scene instance;

	return &instance;
}

Object* Scene::addChild(Object* parentObject)
{
	Object* child = new Object();
	parentObject->node.addChild(&child->node);

	child->ID = idCounter;
	objectsToRender[idCounter] = child;
	idCounter++;
	LastAddedObject = child;

	return child;
}

void Scene::addRandomObject(const Vector3& pos)
{
	Object* newChild = Scene::addChild(SceneObject);

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

Object* Scene::addObject(const char* name, const Vector3& pos)
{
	Object* newChild = Scene::addChild(SceneObject);
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
		Object* obj = addObject(name, generateRandomIntervallVectorCubic(min, max));
	}
}


Object* Scene::addPhysicObject(const char* name, const Vector3& pos)
{
	Object* object = addObject(name, pos);
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
	for (auto& obj : objectsToRender)
	{
		delete obj.second;
	}
	objectsToRender.clear();

	SceneObject->node.children.clear();
	idCounter = 1;

	for (auto& obj : pointLights)
	{
		delete obj;
	}
	pointLights.clear();

	MainPointLight->node.children.clear();

	for (auto& obj : directionalLights)
	{
		delete obj;
	}
	directionalLights.clear();

	MainDirectionalLight->node.children.clear();

	SceneObject->ClearComponents();
}

Object* Scene::addPointLight(const Vector3& position, const Vector3& color)
{
	Object* newChild = new Object();
	MainPointLight->node.addChild(&newChild->node);
	
	newChild->SetPosition(position);

	Material* newMaterial = new Material();
	newMaterial->SetColor(color); 
	newChild->AssignMesh(GraphicsStorage::meshes["sphere"]);
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	newChild->SetScale(Vector3(4.f, 4.f, 4.f));
	pointLights.push_back(newChild);
	return newChild;
}

Object* Scene::addDirectionalLight(const Vector3& direction, const Vector3& color /*= Vector3(1, 1, 1)*/)
{
	Object* newChild = new Object();
	MainDirectionalLight->node.addChild(&newChild->node);

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
	addPointLight(generateRandomIntervallVectorCubic(min, max), generateRandomIntervallVectorCubic(0, 255)/255.f);
}

Vector3 Scene::generateRandomIntervallVectorCubic(int min, int max)
{
	int range = max - min + 1;
	int num = rand() % range + min;

	int rX = rand() % range + min;
	int rY = rand() % range + min;
	int rZ = rand() % range + min;

	return Vector3((float)rX, (float)rY, (float)rZ);
}


mwm::Vector3 Scene::generateRandomIntervallVectorFlat(int min, int max, axis axis, int axisHeight)
{
	int range = max - min + 1;
	int num = rand() % range + min;

	int r1 = rand() % range + min;
	int r2 = rand() % range + min;

	if (axis == x) return Vector3((float)axisHeight, (float)r1, (float)r2);
	else if (axis == y) return Vector3((float)r1, (float)axisHeight, (float)r2);
	else return Vector3((float)r1, (float)r2, (float)axisHeight);
}


void Scene::addRandomlyPointLights(int num, int min, int max)
{
	for (int i = 0; i < num; i++)
	{
		addRandomPointLight(min, max);
	}
}
