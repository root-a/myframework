#include "Scene.h"
#include "Node.h"
#include "Object.h"
#include "Material.h"
#include "GraphicsStorage.h"
#include "PhysicsManager.h"
#include "Mesh.h"
#include "OBJ.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"

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

Object* Scene::addChild()
{
	Object* child = new Object();
	SceneObject->node.addChild(&child->node);

	child->ID = idCounter;
	idCounter++;

	return child;
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
	Object* newChild = Scene::addChild();
	pickingList[newChild->ID] = newChild;
	renderList.push_back(newChild);

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
	Object* newChild = Scene::addChild();
	pickingList[newChild->ID] = newChild;
	renderList.push_back(newChild);

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
	renderList.push_back(newChild);

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
	for (auto& obj : renderList)
	{
		delete obj;
	}
	renderList.clear();

	pickingList.clear();

	SceneObject->node.children.clear();
	idCounter = 1;

	for (auto& obj : pointLights)
	{
		delete obj;
	}
	pointLights.clear();

	for (auto& obj : spotLights)
	{
		delete obj;
	}
	spotLights.clear();

	for (auto& obj : directionalLights)
	{
		delete obj;
	}
	directionalLights.clear();

	directionalLightComponents.clear();
	spotLightComponents.clear();
	pointLightComponents.clear();

	SceneObject->ClearComponents(); //just clear components
}

Object* Scene::addPointLight(bool castShadow, const Vector3& position, const Vector3F& color)
{
	Object* newChild = Scene::addChild();
	PointLight * pointLightComp = new PointLight();
	if (castShadow) pointLightComp->GenerateShadowMapBuffer(1024, 1024);
	newChild->AddComponent(pointLightComp);
	pointLightComponents.push_back(pointLightComp);

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

Object * Scene::addPointLightTo(Object * parent, bool castShadow, const mwm::Vector3 & position, const mwm::Vector3F & color)
{
	Object* newChild = Scene::addChildTo(parent);
	PointLight * pointLightComp = new PointLight();
	if (castShadow) pointLightComp->GenerateShadowMapBuffer(1024, 1024);
	newChild->AddComponent(pointLightComp);
	pointLightComponents.push_back(pointLightComp);

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

Object * Scene::addSpotLight(bool castShadow, const mwm::Vector3 & position, const mwm::Vector3F & color)
{
	Object* newChild = Scene::addChild();
	SpotLight * spotLightComp = new SpotLight(newChild);
	if (castShadow)
	{
		spotLightComp->GenerateShadowMapBuffer();
		spotLightComp->GenerateBlurShadowMapBuffer();
	}
	newChild->AddComponent(spotLightComp);
	spotLightComponents.push_back(spotLightComp);

	newChild->SetPosition(position);
	Material* newMaterial = new Material();
	newMaterial->SetColor(color);
	newChild->AssignMesh(GraphicsStorage::meshes["cone"]);
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	newChild->SetScale(Vector3(4.0, 4.0, 4.0));
	spotLights.push_back(newChild);
	return newChild;
}

Object * Scene::addSpotLightTo(Object * parent, bool castShadow, const mwm::Vector3 & position, const mwm::Vector3F & color)
{
	Object* newChild = Scene::addChildTo(parent);
	SpotLight * spotLightComp = new SpotLight(newChild);
	if (castShadow)
	{
		spotLightComp->GenerateShadowMapBuffer();
		spotLightComp->GenerateBlurShadowMapBuffer();
	}
	newChild->AddComponent(spotLightComp);
	spotLightComponents.push_back(spotLightComp);

	newChild->SetPosition(position);
	Material* newMaterial = new Material();
	newMaterial->SetColor(color);
	newChild->AssignMesh(GraphicsStorage::meshes["cone"]);
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);
	newChild->SetScale(Vector3(4.0, 4.0, 4.0));
	spotLights.push_back(newChild);
	return newChild;
}

Object* Scene::addDirectionalLight(bool castShadow, const Vector3F& color /*= Vector3(1, 1, 1)*/)
{
	Object* newChild = Scene::addChild();
	DirectionalLight* dirLightComp = new DirectionalLight();
	dirLightComp->hasShadowMap = castShadow;
	newChild->AddComponent(dirLightComp);
	directionalLightComponents.push_back(dirLightComp);

	Material* newMaterial = new Material();
	newMaterial->SetColor(color);
	newChild->AssignMesh(GraphicsStorage::meshes["plane"]);
	GraphicsStorage::materials.push_back(newMaterial);
	newChild->AssignMaterial(newMaterial);

	directionalLights.push_back(newChild);

	return newChild;
}

Object * Scene::addDirectionalLightTo(Object * parent, bool castShadow, const mwm::Vector3F & color)
{
	Object* newChild = Scene::addChildTo(parent);
	DirectionalLight* dirLightComp = new DirectionalLight();
	dirLightComp->hasShadowMap = castShadow;
	newChild->AddComponent(dirLightComp);
	directionalLightComponents.push_back(dirLightComp);
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
	SceneObject->node.UpdateNodeTransform(SceneObject->node); //update scenegraph

	//update components:
	SceneObject->Update();
	for (auto& obj : renderList)
	{
		obj->Update();
	}
	for (auto& obj : pointLights)
	{
		obj->Update();
	}
	for (auto& obj : spotLights)
	{
		obj->Update();
	}
	for (auto& obj : directionalLights)
	{
		obj->Update();
	}
}