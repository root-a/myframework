#include "InstanceSystem.h"
#include <algorithm>
#include <GL/glew.h>
#include "Object.h"
#include "Frustum.h"
#include "CameraManager.h"
#include "GraphicsStorage.h"
#include "GraphicsManager.h"
#include "SceneGraph.h"



InstanceSystem::InstanceSystem(int maxCount, OBJ* object)
{
	MaxCount = maxCount;
	LastUsed = 0;
	ActiveCount = 0;
	objectContainer = new Object[maxCount];
	M = new Matrix4F[maxCount];
	objectID = new unsigned int[maxCount];
	materialColorShininess = new Vector4F[maxCount];
	paused = true;
	GraphicsManager::LoadOBJToVAO(object, &vao);
	SetUpGPUBuffers();
}

InstanceSystem::~InstanceSystem()
{
	delete[] objectContainer;
	delete[] M;
	delete[] objectID;
	delete[] materialColorShininess;
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
	ActiveCount = 0;
	for (int i = 0; i < MaxCount; i++)
	{
		Object& object = objectContainer[i];

		if (object.CanDraw() && FrustumManager::Instance()->isBoundingSphereInView(object.bounds->centeredPosition, object.bounds->circumRadius))
		{
			//object.node->UpdateNode(this->object->node);
			M[ActiveCount] = object.node->TopDownTransform.toFloat();
			objectID[ActiveCount] = object.ID;
			//materialColorShininess[ActiveCount] = object.mat->colorShininess;
			ActiveCount += 1;

			object.UpdateDrawState();
		}
	}
}

void InstanceSystem::UpdateCPUBuffersNoCulling()
{
	ActiveCount = 0;
	for (int i = 0; i < MaxCount; i++)
	{
		Object& object = objectContainer[i];

		//object.node->UpdateNode(this->object->node);
		M[ActiveCount] = object.node->TopDownTransform.toFloat();
		objectID[ActiveCount] = object.ID;
		//materialColorShininess[ActiveCount] = object.mat->colorShininess;
		ActiveCount += 1;

		object.UpdateDrawState();
	}
	ActiveCount = std::min(MaxCount, ActiveCount);
}

void InstanceSystem::SetUpGPUBuffers()
{
	modelBuffer = vao.AddVertexBuffer(NULL, MaxCount * sizeof(Matrix4F), { {ShaderDataType::Mat4, "Model", 1} });
	objectIDBuffer = vao.AddVertexBuffer(NULL, MaxCount * sizeof(unsigned int), { {ShaderDataType::Int, "ID", 1} });
	materialColorBuffer = vao.AddVertexBuffer(NULL, MaxCount * sizeof(Vector4F), { {ShaderDataType::Float4, "MaterialColor", 1} });
}

void InstanceSystem::UpdateGPUBuffers()
{
	glNamedBufferSubData(modelBuffer, 0, ActiveCount * sizeof(Matrix4F), M);
	glNamedBufferSubData(objectIDBuffer, 0, ActiveCount * sizeof(unsigned int), objectID);
	glNamedBufferSubData(materialColorBuffer, 0, ActiveCount * sizeof(Vector4F), materialColorShininess);
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
	
	vao.activeCount = ActiveCount;
	vao.Draw();
	
	return ActiveCount;
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
		Material* newMaterial = new Material();
		GraphicsStorage::materials.push_back(newMaterial);
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