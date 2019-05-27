#include "InstanceSystem.h"
#include <algorithm>
#include <GL/glew.h>
#include "Object.h"
#include "Frustum.h"
#include "CameraManager.h"
#include "GraphicsStorage.h"
#include "GraphicsManager.h"
#include "Scene.h"

using namespace mwm;

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
			materialColorShininess[ActiveCount] = object.mat->colorShininess;
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
		materialColorShininess[ActiveCount] = object.mat->colorShininess;
		ActiveCount += 1;

		object.UpdateDrawState();
	}
}

void InstanceSystem::SetUpGPUBuffers()
{
	vao.Bind();

	glGenBuffers(1, &modelBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Matrix4F), NULL, GL_STREAM_DRAW);
	for (unsigned int i = 0; i < 4; i++) {
		glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4F), (const GLvoid*)(sizeof(GLfloat) * i * 4));
		glEnableVertexAttribArray(3 + i);
		glVertexAttribDivisor(3 + i, 1); // model matrices : one per box
	}
	vao.vertexBuffers.push_back(modelBuffer);

	glGenBuffers(1, &objectIDBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, objectIDBuffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(unsigned int), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(7);
	glVertexAttribDivisor(7, 1); // id : one per box
	vao.vertexBuffers.push_back(objectIDBuffer);

	glGenBuffers(1, &materialColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, materialColorBuffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector4F), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(8);
	glVertexAttribDivisor(8, 1); // color : one per box
	vao.vertexBuffers.push_back(materialColorBuffer);

	//Unbind the VAO now that the VBOs have been set up
	vao.Unbind();
}

void InstanceSystem::UpdateGPUBuffers()
{
	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Matrix4F), M);

	glBindBuffer(GL_ARRAY_BUFFER, objectIDBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(unsigned int), objectID);

	glBindBuffer(GL_ARRAY_BUFFER, materialColorBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Vector4F), materialColorShininess);
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
	
	glDrawElementsInstanced(GL_TRIANGLES, vao.indicesCount, GL_UNSIGNED_INT, (void*)0, ActiveCount);
	
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
		objectContainer[i].node->SetScale(Scene::Instance()->generateRandomIntervallVectorSpherical(5, 15));
		objectContainer[i].DrawAlways();
		objectContainer[i].node->SetPosition(Scene::Instance()->generateRandomIntervallVectorSpherical(20, 1000));
		objectContainer[i].bounds->SetUp(parent->bounds->centerOfMesh, parent->bounds->dimensions, parent->bounds->name);
		objectContainer[i].node->UpdateNode(*parent->node);
		Scene::Instance()->registerForPicking(&objectContainer[i]);
	}
	UpdateCPUBuffersNoCulling();
	UpdateGPUBuffers();
}