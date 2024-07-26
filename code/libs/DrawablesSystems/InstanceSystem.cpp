#include "InstanceSystem.h"
#include <algorithm>
#include <GL/glew.h>
#include "Object.h"
#include "Frustum.h"
#include "CameraManager.h"
#include "GraphicsStorage.h"
#include "GraphicsManager.h"
#include "SceneGraph.h"

InstanceSystem::InstanceSystem()
{
	LastUsed = 0;
	paused = true;
	objectContainer = nullptr;
}

InstanceSystem::InstanceSystem(int maxCount, OBJ* object)
{
	MaxCount = maxCount;
	LastUsed = 0;
	objectContainer = new Object[maxCount];
	paused = true;
	GraphicsManager::LoadOBJToVAO(object, &vao);
	SetUpGPUBuffers();
}

InstanceSystem::~InstanceSystem()
{
	delete[] objectContainer;
}

void InstanceSystem::SetUp(int maxCount, OBJ* object)
{
	MaxCount = maxCount;
	LastUsed = 0;
	objectContainer = new Object[maxCount];
	paused = true;
	GraphicsManager::LoadOBJToVAO(object, &vao);
	SetUpGPUBuffers();
}

int InstanceSystem::FindUnused()
{
	for (int i = LastUsed; i < MaxCount; i++){
		if (objectContainer[i].CanDraw() == false){
			LastUsed = i;
			return i;
		}
	}

	for (int i = 0; i < LastUsed; i++){
		if (objectContainer[i].CanDraw() == false){
			LastUsed = i;
			return i;
		}
	}

	return 0;
}

void InstanceSystem::UpdateCPUBuffers()
{
	modelBuffer->activeCount = 0;
	objectIDBuffer->activeCount = 0;
	for (int i = 0; i < MaxCount; i++)
	{
		Object& object = objectContainer[i];

		if (object.CanDraw() && SceneGraph::Instance()->frustum.isBoundingSphereInView(object.bounds->centeredPosition, object.bounds->circumRadius))
		{
			//object.node->UpdateNode(this->object->node);
			modelBuffer->SetData(modelBuffer->activeCount, *model, &object.node->TopDownTransformF);
			modelBuffer->SetData(objectIDBuffer->activeCount, *id, &object.ID);
			//materialColorShininess[ActiveCount] = object.mat->colorShininess;
			modelBuffer->IncreaseInstanceCount();
			objectIDBuffer->IncreaseInstanceCount();

			object.UpdateDrawState();
		}
	}
}

void InstanceSystem::UpdateCPUBuffersNoCulling()
{
	modelBuffer->activeCount = 0;
	objectIDBuffer->activeCount = 0;
	for (int i = 0; i < MaxCount; i++)
	{
		Object& object = objectContainer[i];

		//object.node->UpdateNode(this->object->node);
		modelBuffer->SetData(modelBuffer->activeCount, *model, &object.node->TopDownTransformF);
		modelBuffer->SetData(objectIDBuffer->activeCount, *id, &object.ID);
		//materialColorShininess[ActiveCount] = object.mat->colorShininess;
		modelBuffer->IncreaseInstanceCount();
		objectIDBuffer->IncreaseInstanceCount();

		object.UpdateDrawState();
	}
}

void InstanceSystem::SetUpGPUBuffers()
{
	BufferLayout vbModel({ {ShaderDataType::Type::FloatMat4, "M", 1} });
	BufferLayout vbID({ {ShaderDataType::Type::Int, "ID", 1} });
	BufferLayout vbColor({ {ShaderDataType::Type::Float4, "MaterialColor", 1} });
	
	modelBuffer = GraphicsStorage::assetRegistry.AllocAsset<VertexBufferDynamic>(nullptr, MaxCount, vbModel);
	objectIDBuffer = GraphicsStorage::assetRegistry.AllocAsset<VertexBufferDynamic>(nullptr, MaxCount, vbID);
	materialColorBuffer = GraphicsStorage::assetRegistry.AllocAsset<VertexBufferDynamic>(nullptr, MaxCount, vbColor);
	
	vao.AddVertexBuffer(modelBuffer);
	vao.AddVertexBuffer(objectIDBuffer);
	vao.AddVertexBuffer(materialColorBuffer);
	
	model = &modelBuffer->layout.locations[0];
	id = &objectIDBuffer->layout.locations[1];
	color = &materialColorBuffer->layout.locations[0];
}

void InstanceSystem::UpdateGPUBuffers()
{
	modelBuffer->Update();
	objectIDBuffer->Update();
	//glNamedBufferSubData(materialColorBuffer, 0, ActiveCount * sizeof(Vector4F), materialColorShininess);
}

int InstanceSystem::Draw()
{
	if (dirty)
	{
		UpdateCPUBuffers();
		UpdateGPUBuffers();
		dirty = false;
	}
	vao.Bind();
	vao.activeCount = modelBuffer->activeCount;
	vao.Draw();
	
	return modelBuffer->activeCount;
}

Object* InstanceSystem::GetObject()
{
	Object* bb = &objectContainer[FindUnused()];
	bb->DrawAlways();
	dirty = true;
	return bb;
}

Object* InstanceSystem::GetObjectOnce()
{
	Object* bb = &objectContainer[FindUnused()];
	bb->DrawOnce();
	dirty = true;
	return bb;
}

void InstanceSystem::Update()
{
	if (!paused)
	{
		UpdateCPUBuffers();
		UpdateGPUBuffers();
	}
}

void InstanceSystem::Init(Object * parent)
{
	Component::Init(parent);

	for (size_t i = 0; i < MaxCount; i++)
	{
		Material* newMaterial = GraphicsStorage::assetRegistry.AllocAsset<Material>();
		newMaterial->name = "instSysMat";
		*newMaterial = mat;
		objectContainer[i].AssignMaterial(newMaterial);
		objectContainer[i].node->SetScale(SceneGraph::Instance()->generateRandomIntervallVectorSpherical(5, 15));
		objectContainer[i].DrawAlways();
		objectContainer[i].node->SetPosition(SceneGraph::Instance()->generateRandomIntervallVectorSpherical(20, 1000));
		objectContainer[i].bounds->SetUp(parent->bounds->centerOfMesh, parent->bounds->dimensions, parent->bounds->name);
		objectContainer[i].node->UpdateNode(*parent->node);
		SceneGraph::Instance()->registerForPicking(&objectContainer[i]);
	}
	UpdateCPUBuffersNoCulling();
	UpdateGPUBuffers();
}

Component* InstanceSystem::Clone()
{
	return new InstanceSystem(*this);
}
