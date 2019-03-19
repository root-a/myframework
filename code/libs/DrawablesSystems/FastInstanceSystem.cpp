#include "FastInstanceSystem.h"
#include "Vao.h"
#include "Material.h"
#include <algorithm>
#include <GL/glew.h>
#include "Object.h"
#include "Frustum.h"
#include "CameraManager.h"
#include "GraphicsStorage.h"
#include "Scene.h"

using namespace mwm;

FastInstanceSystem::FastInstanceSystem(int maxCount)
{
	MaxCount = maxCount;
	ActiveCount = 0;
	objectContainer = new Object[maxCount];
	M = new Matrix4F[maxCount];
	objectID = new unsigned int[maxCount];
	materialColor = new Vector3F[maxCount];
	materialProperties = new Vector4F[maxCount];
	paused = false;
	indexMap.reserve(maxCount);
}

FastInstanceSystem::~FastInstanceSystem()
{
	delete[] objectContainer;
	delete[] M;
	delete[] objectID;
	delete[] materialColor;
	delete[] materialProperties;
}

//this is initial
void FastInstanceSystem::UpdateCPUBuffers()
{
	for (int i = 0; i < nrOfGetsAtUpdate; i++)
	{
		Object& object = objectContainer[i];

		M[i] = object.node->TopDownTransform.toFloat();
		objectID[i] = object.ID;
		materialColor[i] = object.mat->color;
		materialProperties[i] = object.mat->properties;
	}
}

void FastInstanceSystem::RetriveObject()
{
	Object& object = objectContainer[ActiveCount];
	M[nrOfGetsAtUpdate] = object.node->TopDownTransform.toFloat();
	objectID[nrOfGetsAtUpdate] = object.ID;
	materialColor[nrOfGetsAtUpdate] = object.mat->color;
	materialProperties[nrOfGetsAtUpdate] = object.mat->properties;
	
	if (ActiveCount < MaxCount) {
		ActiveCount++;
		nrOfGetsAtUpdate++;
	}
}

//this is initial
void FastInstanceSystem::SetUpGPUBuffers()
{
	vao->Bind();

	glGenBuffers(1, &modelBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Matrix4F), NULL, GL_STREAM_DRAW);
	for (unsigned int i = 0; i < 4; i++) {
		glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4F), (const GLvoid*)(sizeof(GLfloat) * i * 4));
		glEnableVertexAttribArray(3 + i);
		glVertexAttribDivisor(3 + i, 1); // model matrices : one per box
	}
	vao->vertexBuffers.push_back(modelBuffer);

	glGenBuffers(1, &objectIDBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, objectIDBuffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(unsigned int), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(7);
	glVertexAttribDivisor(7, 1); // id : one per box
	
	glGenBuffers(1, &materialColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, materialColorBuffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector3F), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(8);
	glVertexAttribDivisor(8, 1); // color : one per box
	vao->vertexBuffers.push_back(materialColorBuffer);
	
	glGenBuffers(1, &materialPropertiesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, materialPropertiesBuffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxCount * sizeof(Vector4F), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(9);
	glVertexAttribDivisor(9, 1); // color : one per box
	vao->vertexBuffers.push_back(materialPropertiesBuffer);
	
	//Unbind the VAO now that the VBOs have been set up
	vao->Unbind();
}

//this is initial
void FastInstanceSystem::UpdateGPUBuffers()
{
	//glBindVertexArray(vao->vaoHandle);

	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Matrix4F), M);

	glBindBuffer(GL_ARRAY_BUFFER, objectIDBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(unsigned int), objectID);

	glBindBuffer(GL_ARRAY_BUFFER, materialColorBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Vector3F), materialColor);

	glBindBuffer(GL_ARRAY_BUFFER, materialPropertiesBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ActiveCount * sizeof(Vector4F), materialProperties);
}

int FastInstanceSystem::Draw()
{
	/*
	if (dirty)
	{
		UpdateCPUBuffers();
		UpdateGPUBuffers();
		dirty = false;
	}
	*/
	vao->Bind();
	
	glDrawElementsInstanced(GL_TRIANGLES, vao->indicesCount, GL_UNSIGNED_INT, (void*)0, ActiveCount);
	
	return ActiveCount;
}

//this is for runtime
Object* FastInstanceSystem::GetObject()
{
	Object& object = objectContainer[ActiveCount];
	indexMap[&object] = ActiveCount;
	if (ActiveCount < MaxCount) ActiveCount++;
	dirty = true;
	nrOfGetsAtUpdate++;
	return &object;
}

//this is for runtime
void FastInstanceSystem::ReturnObject(Object* object)
{
	std::unordered_map<Object*, int>::iterator it = indexMap.find(object);
	if (it == indexMap.end()) return;

	int index = indexMap[object];

	ActiveCount--;
	Object& lastObject = objectContainer[ActiveCount];
	//Scene::Instance()->unregisterForPicking(object);
	indexMap[object] = ActiveCount;
	indexMap[&lastObject] = index;
	//update gpu buffers at index
	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, index*sizeof(Matrix4F), sizeof(Matrix4F), &lastObject.node->TopDownTransform.toFloat());

	glBindBuffer(GL_ARRAY_BUFFER, objectIDBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, index* sizeof(unsigned int), sizeof(unsigned int), &lastObject.ID);

	glBindBuffer(GL_ARRAY_BUFFER, materialColorBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, index* sizeof(Vector3F), sizeof(Vector3F), &lastObject.mat->color);

	glBindBuffer(GL_ARRAY_BUFFER, materialPropertiesBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, index* sizeof(Vector4F), sizeof(Vector4F), &lastObject.mat->properties);
	
}

//this is for runtime
void FastInstanceSystem::UpdateObject(Object* object)
{
	std::unordered_map<Object*, int>::iterator it = indexMap.find(object);
	if (it == indexMap.end()) return;

	int index = indexMap[object];
	Object& objectc = objectContainer[index];

	//update gpu buffers at index
	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(Matrix4F), sizeof(Matrix4F), &objectc.node->TopDownTransform.toFloat());

	glBindBuffer(GL_ARRAY_BUFFER, objectIDBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(unsigned int), sizeof(unsigned int), &objectc.ID);

	glBindBuffer(GL_ARRAY_BUFFER, materialColorBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(Vector3F), sizeof(Vector3F), &objectc.mat->color);

	glBindBuffer(GL_ARRAY_BUFFER, materialPropertiesBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(Vector4F), sizeof(Vector4F), &objectc.mat->properties);
}

void FastInstanceSystem::UpdateObjects()
{
	if (nrOfGetsAtUpdate == 0) return;

	int startIndexToUpdate = ActiveCount - nrOfGetsAtUpdate;
	
	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, startIndexToUpdate * sizeof(Matrix4F), nrOfGetsAtUpdate * sizeof(Matrix4F), M);

	glBindBuffer(GL_ARRAY_BUFFER, objectIDBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, startIndexToUpdate * sizeof(unsigned int), nrOfGetsAtUpdate * sizeof(unsigned int), objectID);

	glBindBuffer(GL_ARRAY_BUFFER, materialColorBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, startIndexToUpdate * sizeof(Vector3F), nrOfGetsAtUpdate * sizeof(Vector3F), materialColor);

	glBindBuffer(GL_ARRAY_BUFFER, materialPropertiesBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, startIndexToUpdate * sizeof(Vector4F), nrOfGetsAtUpdate * sizeof(Vector4F), materialProperties);

	nrOfGetsAtUpdate = 0;
}

void FastInstanceSystem::ReturnObjects()
{
	std::unordered_map<Object*, int>::iterator it = indexMap.find(object);
	if (it == indexMap.end()) return;

	int index = indexMap[object];

	ActiveCount--;
	Object& lastObject = objectContainer[ActiveCount];
	//Scene::Instance()->unregisterForPicking(object);
	indexMap[object] = ActiveCount;
	indexMap[&lastObject] = index;
	//update gpu buffers at index
	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(Matrix4F), sizeof(Matrix4F), &lastObject.node->TopDownTransform.toFloat());

	glBindBuffer(GL_ARRAY_BUFFER, objectIDBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(unsigned int), sizeof(unsigned int), &lastObject.ID);

	glBindBuffer(GL_ARRAY_BUFFER, materialColorBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(Vector3F), sizeof(Vector3F), &lastObject.mat->color);

	glBindBuffer(GL_ARRAY_BUFFER, materialPropertiesBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(Vector4F), sizeof(Vector4F), &lastObject.mat->properties);
}

void FastInstanceSystem::Update()
{
	/*
	if (!paused)
	{
		UpdateCPUBuffers();
		UpdateGPUBuffers();
	}
	*/
}

void FastInstanceSystem::Init(Object * parent)
{
	Component::Init(parent);
	vao = parent->vao;
	mat = parent->mat;
	SetUpGPUBuffers();
	for (size_t i = 0; i < MaxCount; i++)
	{
		Material* newMaterial = new Material();
		GraphicsStorage::materials.push_back(newMaterial);
		*newMaterial = *parent->mat;
		objectContainer[i].AssignMaterial(newMaterial);
		objectContainer[i].node->SetScale(Scene::Instance()->generateRandomIntervallVectorSpherical(1, 15));
		objectContainer[i].node->SetPosition(Scene::Instance()->generateRandomIntervallVectorSpherical(20, 1000));
		objectContainer[i].bounds->SetUp(parent->bounds->centerOfMesh, parent->bounds->dimensions, parent->bounds->name);
		objectContainer[i].node->UpdateNode(*parent->node);
		Scene::Instance()->registerForPicking(&objectContainer[i]);
		
	}
	for (size_t i = 0; i < 6003; i++)
	{
		GetObject();
	}
	UpdateCPUBuffers();
	UpdateObjects();

	//UpdateCPUBuffers(); //this is initial
	//UpdateGPUBuffers(); //this is initial
}
//WIP


//we can write a function to update the range
//we don't have to send in any arguments
//we just need to internally keep track of number of getted objects
//the first getted object since first update and the last to get the number
//then we can update a range of objects
//we can do update per frame this way in the component just by checking first the number of submitted objects

//current update object allows for selective update

//we could do all this with existing scenegraph objects
//submitting the objects, not having a container in the component
//or we could store them in the bank

//yes we can make returning possible
//we just register indexes of objects we want to return,
//then when we update we do two operations, we remove all returned and we add all requested boom!
