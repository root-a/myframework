#include "SceneGraph.h"
#include "Node.h"
#include "Object.h"
#include "Material.h"
#include "TextureProfile.h"
#include "MaterialProfile.h"
#include "GraphicsStorage.h"
#include "PhysicsManager.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "InstanceSystem.h"
#include "FastInstanceSystem.h"
#include <chrono>
#include "Frustum.h"
#include "TextureProfile.h"
#include "ScriptsComponent.h"
#include "BoundingBoxSystem.h"
#include "LineSystem.h"
#include "PointSystem.h"
#include "RenderPass.h"
#include "RenderProfile.h"
#include "Texture.h"
#include "ParticleSystem.h"
#include "ObjectProfile.h"
#include "CircleSystem.h"
#include "OBJ.h"
#include "Script.h"

SceneGraph::SceneGraph()
{
	//SceneObject = nullptr;
	dirtyDynamicArray = false;
	SceneRoot.name = "root";
	SceneRoot.SetMovable(false);
	ReInit();
}

SceneGraph::~SceneGraph()
{
	
}

void SceneGraph::ReInit()
{
	GraphicsStorage::assetRegistry.RegisterType<Object>();
	GraphicsStorage::assetRegistry.RegisterType<Bounds>();
	GraphicsStorage::assetRegistry.RegisterType<Node>();
	GraphicsStorage::assetRegistry.RegisterType<RigidBody>();
	GraphicsStorage::assetRegistry.RegisterType<DirectionalLight>();
	GraphicsStorage::assetRegistry.RegisterType<SpotLight>();
	GraphicsStorage::assetRegistry.RegisterType<PointLight>();
	GraphicsStorage::assetRegistry.RegisterType<InstanceSystem>();
	GraphicsStorage::assetRegistry.RegisterType<FastInstanceSystem>();
	GraphicsStorage::assetRegistry.RegisterType<ScriptsComponent>();
	GraphicsStorage::assetRegistry.RegisterType<BoundingBoxSystem>();
	GraphicsStorage::assetRegistry.RegisterType<LineSystem>();
	GraphicsStorage::assetRegistry.RegisterType<PointSystem>();
	GraphicsStorage::assetRegistry.RegisterType<CircleSystem>();
	GraphicsStorage::assetRegistry.RegisterType<ParticleSystem>();
	GraphicsStorage::assetRegistry.RegisterType<ObjectProfile>();

	GraphicsStorage::assetRegistry.RegisterType<Material>();
	GraphicsStorage::assetRegistry.RegisterType<Script>();

	/*
	auto afTexture = GraphicsStorage::assetRegistry.GetFactory<Texture>();
	for (auto& chunk : afTexture->pool)
	{
		for (auto& tex : chunk)
		{
			auto id = afTexture->GetAssetID(&tex);
			printf("texture %s has id %d\n", tex.name.c_str(), id);
		}
	}

	printf("for auto loop\n");
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;

	start = std::chrono::high_resolution_clock::now();
	int j = 0;
	for (auto& chunk : objectPool->pool)
	{
		//printf("chunk nr %d\n", j);
		for (auto& asset : chunk)
		{
			//printf("object id in a chunk %d\n", asset.ID);
			j++;
		}
		j++;
	}
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	double forLoopTime = elapsed_seconds.count();
	printf("for loop took %f seconds\n", forLoopTime);
	printf("for iterator loop\n");
	start = std::chrono::high_resolution_clock::now();
	j = 0;
	for (auto& it = objectPool->pool.begin(), &end = objectPool->pool.end(); it != end; ++it) {
		auto& i = *it;
		//printf("chunk nr %d\n", j);
		for (auto& o = i.begin(), &e = i.end(); o != e; ++o) {
			const auto& oref = *o;
			//printf("object id in a chunk %d\n", oref.ID);
			j++;
		}
		j++;
	}
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	double iteratorLoop = elapsed_seconds.count();
	printf("iterator loop took %f seconds\n", iteratorLoop);
	*/
}

SceneGraph* SceneGraph::Instance()
{
	static SceneGraph instance;

	return &instance;
}

Object* SceneGraph::addSimpleObject()
{
	Object* child = GraphicsStorage::assetRegistry.AllocAsset<Object>();
	allObjects.push_back(child);
	return child;
}

Object* SceneGraph::addObject(Object* object)
{
	allObjects.push_back(object);
	return object;
}

Object* SceneGraph::addChild()
{
	return addChildTo(&SceneRoot);
}

Object* SceneGraph::addChildTo(Node* parentNode)
{
	Object* child = GraphicsStorage::assetRegistry.AllocAsset<Object>();
	allObjects.push_back(child);
	child->node = GraphicsStorage::assetRegistry.AllocAsset<Node>();
	child->AddComponent(child->node);
	parentNode->addChild(child->node);
	return child;
}

void SceneGraph::CreateSceneObject()
{
	Object* scene = GraphicsStorage::assetRegistry.AllocAsset<Object>();
	scene->name = "scene";
	scene->node = GraphicsStorage::assetRegistry.AllocAsset<Node>();
	scene->AddComponent(scene->node);
	allObjects.push_back(scene);
}

Material* SceneGraph::CreateDefaultMaterial()
{
	Material* newMaterial = GraphicsStorage::assetRegistry.AllocAsset<Material>();
	TextureProfile* texProfile = GraphicsStorage::assetRegistry.AllocAsset<TextureProfile>();
	newMaterial->AssignTextureProfile(texProfile);
	for (auto& texture : *GraphicsStorage::assetRegistry.GetPool<Texture>())
	{
		if (texture.name == "default_diffuse")
		{
			newMaterial->tp->AddTexture(&texture, 0);
		}
		else if (texture.name == "default_normal_dtx5")
		{
			newMaterial->tp->AddTexture(&texture, 1);
		}
		else if (texture.name == "default_aometrough")
		{
			newMaterial->tp->AddTexture(&texture, 2);
		}
	}
	return newMaterial;
}

void SceneGraph::addRandomObject(const glm::vec3& pos)
{
	Object* newChild = SceneGraph::addChild();
	pickingList[newChild->ID] = newChild;
	renderList.push_back(newChild);

	newChild->node->SetPosition(pos);
	float rS = (float)(rand() % 5);
	newChild->node->SetScale(glm::vec3(rS, rS, rS));

	PoolParty<VertexArray>* vaos = GraphicsStorage::assetRegistry.GetPool<VertexArray>();
	int index = rand() % (vaos->GetCount());
	auto it = vaos->begin();
	std::advance(it, index);
	
	newChild->bounds = GraphicsStorage::assetRegistry.AllocAsset<Bounds>((*it).center, (*it).dimensions, (*it).name);
	newChild->AddComponent(newChild->bounds, true);
	Material* newMaterial = CreateDefaultMaterial();
	newMaterial->AssignMesh(&(*it));
	newChild->AssignMaterial(newMaterial);
}

Object* SceneGraph::addInstanceSystem(const char * name, int count, const glm::vec3& pos)
{
	return addInstanceSystemTo(&SceneRoot, name, count, pos);
}

Object* SceneGraph::addInstanceSystemTo(Node* parent, const char * name, int count, const glm::vec3& pos)
{
	Object* newChild = SceneGraph::addChildTo(parent);
	pickingList[newChild->ID] = newChild;
	renderList.push_back(newChild);

	newChild->node->SetPosition(pos);
	Material* newMaterial = CreateDefaultMaterial();
	for (auto& vao : *GraphicsStorage::assetRegistry.GetPool<VertexArray>())
	{
		if (vao.name.compare(name) == 0)
		{
			newChild->bounds = GraphicsStorage::assetRegistry.AllocAsset<Bounds>(vao.center, vao.dimensions, vao.name);
			newChild->AddComponent(newChild->bounds, true);
			newMaterial->AssignMesh(&vao);
			break;
		}
	}
	
	newChild->AssignMaterial(newMaterial);
	for (auto& obj : *GraphicsStorage::assetRegistry.GetPool<OBJ>())
	{
		if (obj.name.compare(name) == 0)
		{
			InstanceSystem* iSystem = GraphicsStorage::assetRegistry.AllocAsset<InstanceSystem>(count, &obj);
			newChild->AddComponent(iSystem, true);
			break;
		}
	}
	
	return newChild;
}

Object* SceneGraph::addFastInstanceSystem(const char * name, int count, const glm::vec3& pos)
{
	return addFastInstanceSystemTo(&SceneRoot, name, count, pos);
}

Object* SceneGraph::addFastInstanceSystemTo(Node* parent, const char* name, int count, const glm::vec3& pos)
{
	Object* newChild = SceneGraph::addChildTo(parent);
	pickingList[newChild->ID] = newChild;
	renderList.push_back(newChild);

	newChild->node->SetPosition(pos);
	Material* newMaterial = CreateDefaultMaterial();

	for (auto& vao : *GraphicsStorage::assetRegistry.GetPool<VertexArray>())
	{
		if (vao.name.compare(name) == 0)
		{
			newChild->bounds = GraphicsStorage::assetRegistry.AllocAsset<Bounds>(vao.center, vao.dimensions, vao.name);
			newChild->AddComponent(newChild->bounds, true);
			newMaterial->AssignMesh(&vao);
			break;
		}
	}

	newChild->AssignMaterial(newMaterial);

	for (auto& obj : *GraphicsStorage::assetRegistry.GetPool<OBJ>())
	{
		if (obj.name.compare(name) == 0)
		{
			FastInstanceSystem* iSystem = GraphicsStorage::assetRegistry.AllocAsset<FastInstanceSystem>(count, &obj);
			newChild->AddComponent(iSystem, true);
			break;
		}
	}
	
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

Object* SceneGraph::addObject(const char* name, const glm::vec3& pos)
{
	return addObjectTo(&SceneRoot, name, pos);
}

Object* SceneGraph::addObjectTo(Node* parent, const char* name /*= "cube"*/, const glm::vec3& pos /*= Vector3()*/)
{
	Object* newChild = SceneGraph::addChildTo(parent);
	newChild->name = name;
	pickingList[newChild->ID] = newChild;
	renderList.push_back(newChild);
	newChild->node->SetPosition(pos);
	Material* newMaterial = CreateDefaultMaterial();

	for (auto& vao : *GraphicsStorage::assetRegistry.GetPool<VertexArray>())
	{
		if (vao.name.compare(name) == 0)
		{
			newChild->bounds = GraphicsStorage::assetRegistry.AllocAsset<Bounds>(vao.center, vao.dimensions, vao.name);
			newChild->AddComponent(newChild->bounds, true);
			newMaterial->AssignMesh(&vao);
			break;
		}
	}

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

Object* SceneGraph::addPhysicObject(const char* name, const glm::vec3& pos)
{
	Object* object = addObject(name, pos);
	object->SetComponentDynamicState(object->node, true);
	//SwitchObjectMovableMode(object, true);
	RigidBody* body = GraphicsStorage::assetRegistry.AllocAsset<RigidBody>();
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
	Object::ResetIDs();
	dynamicNodeArray.clear();
	dirtyDynamicNodes.clear();
	dirtyStaticNodes.clear();
	allObjects.clear();
	renderList.clear();
	pickingList.clear();

	pointLights.clear();
	spotLights.clear();
	directionalLights.clear();

	GraphicsStorage::assetRegistry.ClearType<Object>();
	GraphicsStorage::assetRegistry.ClearType<Bounds>();
	GraphicsStorage::assetRegistry.ClearType<Node>();
	GraphicsStorage::assetRegistry.ClearType<RigidBody>();
	GraphicsStorage::assetRegistry.ClearType<DirectionalLight>();
	GraphicsStorage::assetRegistry.ClearType<SpotLight>();
	GraphicsStorage::assetRegistry.ClearType<PointLight>();
	GraphicsStorage::assetRegistry.ClearType<InstanceSystem>();
	GraphicsStorage::assetRegistry.ClearType<FastInstanceSystem>();
	GraphicsStorage::assetRegistry.ClearType<ScriptsComponent>();
	GraphicsStorage::assetRegistry.ClearType<BoundingBoxSystem>();
	GraphicsStorage::assetRegistry.ClearType<LineSystem>();
	GraphicsStorage::assetRegistry.ClearType<PointSystem>();
	GraphicsStorage::assetRegistry.ClearType<CircleSystem>();
	GraphicsStorage::assetRegistry.ClearType<ParticleSystem>();

	GraphicsStorage::assetRegistry.ClearType<Material>();
	GraphicsStorage::assetRegistry.ClearType<Script>();

	GraphicsStorage::assetRegistry.ClearType<RenderPass>();
	GraphicsStorage::assetRegistry.ClearType<RenderProfile>();
	GraphicsStorage::assetRegistry.ClearType<TextureProfile>();
	GraphicsStorage::assetRegistry.ClearType<MaterialProfile>();
	GraphicsStorage::assetRegistry.ClearType<ObjectProfile>();
}

Object* SceneGraph::addPointLight(bool castShadow, const glm::vec3& position, const glm::vec3& color)
{
	return addPointLightTo(&SceneRoot, castShadow, position, color);
}

Object* SceneGraph::addPointLightTo(Node* parent, bool castShadow, const glm::vec3& position, const glm::vec3& color)
{
	Object* newChild = SceneGraph::addChildTo(parent);
	PointLight* pointLightComp = GraphicsStorage::assetRegistry.AllocAsset<PointLight>();
	if (castShadow) pointLightComp->GenerateShadowMapBuffer(1024, 1024);
	newChild->AddComponent(pointLightComp, true);

	newChild->node->SetPosition(position);
	Material* newMaterial = GraphicsStorage::assetRegistry.AllocAsset<Material>();
	for (auto& vao : *GraphicsStorage::assetRegistry.GetPool<VertexArray>())
	{
		if (vao.name.compare("sphere") == 0)
		{
			newChild->bounds = GraphicsStorage::assetRegistry.AllocAsset<Bounds>(vao.center, vao.dimensions, vao.name);
			newChild->AddComponent(newChild->bounds, true);
			newMaterial->AssignMesh(&vao);
			break;
		}
	}

	newChild->AssignMaterial(newMaterial);
	newChild->node->SetScale(glm::vec3(4.0, 4.0, 4.0));
	pointLights.push_back(newChild);
	return newChild;
}

Object* SceneGraph::addSpotLight(bool castShadow, const glm::vec3& position, const glm::vec3& color)
{
	return addSpotLightTo(&SceneRoot, castShadow, position, color);
}

Object* SceneGraph::addSpotLightTo(Node* parent, bool castShadow, const glm::vec3& position, const glm::vec3& color)
{
	Object* newChild = SceneGraph::addChildTo(parent);
	SpotLight * spotLightComp = GraphicsStorage::assetRegistry.AllocAsset<SpotLight>();
	if (castShadow)
	{
		spotLightComp->GenerateShadowMapBuffer();
		spotLightComp->GenerateBlurShadowMapBuffer();
	}
	newChild->AddComponent(spotLightComp, true);

	newChild->node->SetPosition(position);
	Material* newMaterial = GraphicsStorage::assetRegistry.AllocAsset<Material>();
	for (auto& vao : *GraphicsStorage::assetRegistry.GetPool<VertexArray>())
	{
		if (vao.name.compare("cone") == 0)
		{
			newChild->bounds = GraphicsStorage::assetRegistry.AllocAsset<Bounds>(vao.center, vao.dimensions, vao.name);
			newChild->AddComponent(newChild->bounds, true);
			newMaterial->AssignMesh(&vao);
			break;
		}
	}
	newChild->AssignMaterial(newMaterial);
	newChild->node->SetScale(glm::vec3(4.0, 4.0, 4.0));
	spotLights.push_back(newChild);
	return newChild;
}

Object* SceneGraph::addDirectionalLight(bool castShadow, const glm::vec3& color /*= Vector3(1, 1, 1)*/)
{
	return addDirectionalLightTo(&SceneRoot, castShadow, color);
}

Object* SceneGraph::addDirectionalLightTo(Node* parent, bool castShadow, const glm::vec3& color)
{
	Object* newChild = SceneGraph::addChildTo(parent);
	DirectionalLight* dirLightComp = GraphicsStorage::assetRegistry.AllocAsset<DirectionalLight>();
	dirLightComp->shadowMapActive = castShadow;
	newChild->AddComponent(dirLightComp, true);
	Material* newMaterial = GraphicsStorage::assetRegistry.AllocAsset<Material>();
	for (auto& vao : *GraphicsStorage::assetRegistry.GetPool<VertexArray>())
	{
		if (vao.name.compare("plane") == 0)
		{
			newChild->bounds = GraphicsStorage::assetRegistry.AllocAsset<Bounds>(vao.center, vao.dimensions, vao.name);
			newChild->AddComponent(newChild->bounds, true);
			newMaterial->AssignMesh(&vao);
			newMaterial->unbound = true;
			break;
		}
	}
	newChild->AssignMaterial(newMaterial);

	directionalLights.push_back(newChild);

	return newChild;
}

void SceneGraph::addRandomPointLight(int min, int max)
{
	addPointLight(false, generateRandomIntervallVectorCubic(min, max), generateRandomIntervallVectorCubic(0, 255)/255.f);
}

void SceneGraph::addRandomlyPointLights(int num, int min, int max)
{
	for (int i = 0; i < num; i++)
	{
		addRandomPointLight(min, max);
	}
}

glm::vec3 SceneGraph::generateRandomIntervallVectorCubic(int min, int max)
{
	int range = max - min + 1;

	int rX = rand() % range + min;
	int rY = rand() % range + min;
	int rZ = rand() % range + min;

	return glm::vec3((double)rX, (double)rY, (double)rZ);
}

glm::vec3 SceneGraph::generateRandomIntervallVectorSpherical(int min, int max)
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
	return glm::vec3(rX, rY, rZ);
}

void SceneGraph::Parent(Node* child, Node* newParent)
{
	bool stateChanged = child->Parent(newParent);
	UpdateDynamicAndStaticArrays(child, child->GetMovable(), stateChanged);
}

void SceneGraph::ParentWithOffset(Node* child, Node* newParent, const glm::vec3& newLocalPos, const glm::quat& newLocalOri, const glm::vec3& newLocalScale)
{
	bool stateChanged = child->ParentWithOffset(newParent, newLocalPos, newLocalOri, newLocalScale);
	UpdateDynamicAndStaticArrays(child, child->GetMovable(), stateChanged);
}

void SceneGraph::ParentWithOffset(Node* child, Node* newParent, const glm::mat4& newLocalTransform)
{
	bool stateChanged = child->ParentWithOffset(newParent, newLocalTransform);
	UpdateDynamicAndStaticArrays(child, child->GetMovable(), stateChanged);
}

void SceneGraph::ParentInPlace(Node* child, Node* newParent)
{
	bool stateChanged = child->ParentInPlace(newParent);
	UpdateDynamicAndStaticArrays(child, child->GetMovable(), stateChanged);
}

void SceneGraph::Unparent(Node* child)
{
	bool stateChanged = child->Unparent();
	UpdateDynamicAndStaticArrays(child, child->GetMovable(), stateChanged);
}

void SceneGraph::Unparent(Node* child, Node* newParent)
{
	bool stateChanged = child->Unparent(newParent);
	UpdateDynamicAndStaticArrays(child, child->GetMovable(), stateChanged);
}

void SceneGraph::UnparentInPlace(Node* child)
{
	bool stateChanged = child->UnparentInPlace();
	UpdateDynamicAndStaticArrays(child, child->GetMovable(), stateChanged);
}

void SceneGraph::UnparentInPlace(Node* child, Node* newParent)
{
	bool stateChanged = child->UnparentInPlace(newParent);
	UpdateDynamicAndStaticArrays(child, child->GetMovable(), stateChanged);
}

glm::vec3 SceneGraph::generateRandomIntervallVectorFlat(int min, int max, axis axis, int axisHeight)
{
	int range = max - min + 1;

	int r1 = rand() % range + min;
	int r2 = rand() % range + min;

	if (axis == axis::x) return glm::vec3((double)axisHeight, (double)r1, (double)r2);
	else if (axis == axis::y) return glm::vec3((double)r1, (double)axisHeight, (double)r2);
	else return glm::vec3((double)r1, (double)r2, (double)axisHeight);
}

// TODO
// generalize frustum culling so that we can reuse it for different things
void SceneGraph::FrustumCulling()
{
	objectsInFrustum.clear();
	//problem with this now is that
	// 1 by default inFrustum is false
	// 2 objects that don't have bounds are not added to objectsInFrustum
	// meaning they will not render :(
	// we might need another container that is for objects that always render
	//
	/*
	for (auto& bc : GraphicsStorage::boundsComponents)
	{
		bc.second->object->inFrustum = FrustumManager::Instance()->isBoundingSphereInView(bc.second->centeredPosition, bc.second->circumRadius);
		if (bc.second->object->inFrustum) objectsInFrustum.push_back(bc.second->object);
	}
	*/
	
	for (auto* object : allObjects)
	{
		Bounds* bounds = object->GetComponent<Bounds>(); // we should have list of all bounds components
		if (bounds != nullptr) //checking existing pointer is faster or getting it from the map but whatever!
		{
			object->inFrustum = frustum.isBoundingSphereInView(bounds->centeredPosition, bounds->circumRadius);
			if (object->inFrustum) objectsInFrustum.push_back(object);
		}
		else
		{
			object->inFrustum = true;
			objectsInFrustum.push_back(object);
		}
	}
}

void SceneGraph::BuildDynamicNodeArray()
{
	//we could first search for all root nodes
	/*
	for (auto node : rootNodes)
	{
		if (node->GetMovable())
			dynamicNodeArray.push_back(node);
		else
			SearchNodeForMovables(node);
	}
	*/
	dynamicNodeArray.clear();
	if (SceneRoot.GetMovable())
		dynamicNodeArray.push_back(&SceneRoot);
	else
		SearchNodeForMovables(&SceneRoot);
}

void SceneGraph::SearchNodeForMovables(Node* nodeToSearch)
{
	for (auto childNode : nodeToSearch->children)
	{
		if (childNode->GetMovable())
		{
			dynamicNodeArray.push_back(childNode);
			printf("%d %p\n", count, childNode);
			count += 1;
		}
		else
			SearchNodeForMovables(childNode);
	}
}

void SceneGraph::SwitchObjectMovableMode(Object* object, bool movable) //this will have to be changed so we won't have to call this function but the function on the node itself
{
	SwitchNodeMovableMode(object->node, movable);
}

//we store which nodes are dynamic in our array
//we store only the differentiating nodes
//if we remove totalMovable and leave all this to SceneGraph
//when we want to set node to dynamic
//we will always have to look if ancestors are in dynamic array because if they do then we don't want to add that node to dynamic array
//if ancestors aren't in the array then we can add it
//same if we want to make node static
//we have to check if any of the parents is dynamic
//if not then we can remove this node from dynamic array
//when we have total movable we can just check for parent totalmovable
void SceneGraph::SwitchNodeMovableMode(Node* node, bool movable)
{
	bool stateChanged = node->SetMovable(movable);
	UpdateDynamicAndStaticArrays(node, movable, stateChanged);
}

void SceneGraph::UpdateDynamicAndStaticArrays(Node* node, bool movable, bool stateChanged)
{
	if (node->parent != nullptr)
	{
		if (stateChanged)
		{
			if (movable)
			{
				for (int i = dirtyDynamicNodes.size() - 1; i > -1 ; i--)
				{
					if (node->IsAncestorOf((dirtyDynamicNodes[i])))
					{
						dirtyDynamicNodes[i] = dirtyDynamicNodes.back();
						dirtyDynamicNodes.pop_back();
					}
				}
				dirtyDynamicNodes.push_back(node);
			}
			else
				dirtyStaticNodes.push_back(node);
		}
	}
	else
	{
		int dynamicNodeIndex = FindNodeIndexInDynamicArray(node);
		if (dynamicNodeIndex != -1)
		{
			dynamicNodeArray[dynamicNodeIndex] = dynamicNodeArray.back();
			dynamicNodeArray.pop_back();
		}
	}
}

int SceneGraph::FindNodeIndexInDynamicArray(Node* node)
{
	for (size_t i = 0; i < dynamicNodeArray.size(); i++)
	{
		if (dynamicNodeArray[i] == node)
		{
			return (int)i;
		}
	}
	return -1;
}

void SceneGraph::InitializeSceneTree()
{
	/*
	for (auto node : rootNodes)
	{
		node->UpdateNode(Node::worldNode);
	}
	*/
	SceneRoot.UpdateNode(Node());
	BuildDynamicNodeArray();
}

void SceneGraph::Update()
{
	Bounds::updateBoundsTime = 0.0;
	Bounds::updateMinMaxTime = 0.0;
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;

	start = std::chrono::high_resolution_clock::now();
	//this is already so complex it might not actually save performance and
	//building new dynamic array on a change might be better, it does not require handling of any edge cases, bug free
	//it would be just a bit slow
	//maybe we could put it on a separate thread?
	//then a switch from dynamic to static could potentially take frames and vice versa hmmm
	for (auto node : dirtyDynamicNodes)
	{
		for (int i = dynamicNodeArray.size() - 1; i > -1; i--)
		{
			if (node->IsAncestorOf((dynamicNodeArray[i])))
			{
				dynamicNodeArray[i] = dynamicNodeArray.back();
				dynamicNodeArray.pop_back();
			}
		}

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
		if (!node->GetTotalMovable() || (!node->GetMovable() && node->parent->GetTotalMovable()))
		{
			int dynamicNodeIndex = FindNodeIndexInDynamicArray(node);
			if (dynamicNodeIndex != -1)
			{
				dynamicNodeArray[dynamicNodeIndex] = dynamicNodeArray.back();
				dynamicNodeArray.pop_back();
				SearchNodeForMovables(node);
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
	
	for (auto object : allObjects)
	{
		object->Update();
		object->UpdateComponents();
	}
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	updateComponentsTime = elapsed_seconds.count();
}