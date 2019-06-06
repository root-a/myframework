#pragma once
#include <GL/glew.h>
#include "DebugDraw.h"
#include "Object.h"
#include "Node.h"
#include "GraphicsStorage.h"
#include "Material.h"
#include "ShaderManager.h"
#include "Render.h"
#include "CameraManager.h"
#include "Camera.h"
#include "Texture.h"
#include "BoundingBoxSystem.h"
#include "PointSystem.h"
#include "LineSystem.h"
#include "FrameBuffer.h"
#include "Frustum.h"
#include "Scene.h"
#include "RigidBody.h"
#include "FBOManager.h"
#include "OBJ.h"
#include "BoundingBox.h"
#include "Line.h"
#include "Plane.h"
#include "Point.h"
#include "Box.h"

using namespace mwm;

DebugDraw::DebugDraw()
{
}

DebugDraw::~DebugDraw()
{
}

DebugDraw* DebugDraw::Instance()
{
	static DebugDraw instance;

	return &instance;
}

void DebugDraw::LoadPrimitives()
{
	debugMat = new Material();
	debugMat->AssignTexture(GraphicsStorage::textures.at(0));

	Object* newObject = new Object();
	newObject->AssignMaterial(debugMat);
	newObject->AssignMesh(GraphicsStorage::vaos["tetra"]);
	newObject->bounds->SetUp(GraphicsStorage::objs["tetra"]->center_of_mesh, GraphicsStorage::objs["tetra"]->dimensions, GraphicsStorage::objs["tetra"]->name);
	debugShapes["tetra"] = newObject;

	newObject = new Object();
	newObject->AssignMaterial(debugMat);
	newObject->AssignMesh(GraphicsStorage::vaos["pyramid"]);
	newObject->bounds->SetUp(GraphicsStorage::objs["pyramid"]->center_of_mesh, GraphicsStorage::objs["pyramid"]->dimensions, GraphicsStorage::objs["pyramid"]->name);
	debugShapes["pyramid"] = newObject;

	newObject = new Object();
	newObject->AssignMaterial(debugMat);
	newObject->AssignMesh(GraphicsStorage::vaos["cube"]);
	newObject->bounds->SetUp(GraphicsStorage::objs["cube"]->center_of_mesh, GraphicsStorage::objs["cube"]->dimensions, GraphicsStorage::objs["cube"]->name);
	debugShapes["cube"] = newObject;

	newObject = new Object();
	newObject->AssignMaterial(debugMat);
	newObject->AssignMesh(GraphicsStorage::vaos["sphere"]);
	newObject->bounds->SetUp(GraphicsStorage::objs["sphere"]->center_of_mesh, GraphicsStorage::objs["sphere"]->dimensions, GraphicsStorage::objs["sphere"]->name);
	debugShapes["sphere"] = newObject;

	newObject = new Object();
	newObject->AssignMaterial(debugMat);
	newObject->AssignMesh(GraphicsStorage::vaos["icosphere"]);
	newObject->bounds->SetUp(GraphicsStorage::objs["icosphere"]->center_of_mesh, GraphicsStorage::objs["icosphere"]->dimensions, GraphicsStorage::objs["icosphere"]->name);
	debugShapes["icosphere"] = newObject;

	newObject = new Object();
	newObject->AssignMaterial(debugMat);
	newObject->AssignMesh(GraphicsStorage::vaos["unitCube"]);
	newObject->bounds->SetUp(GraphicsStorage::objs["unitCube"]->center_of_mesh, GraphicsStorage::objs["unitCube"]->dimensions, GraphicsStorage::objs["unitCube"]->name);
	debugShapes["unitCube"] = newObject;
}

void DebugDraw::DrawShapeAtPos(const char* shapeName, const Vector3& pos)
{
	Object* shape = DebugDraw::Instance()->debugShapes[shapeName];
	shape->node->SetPosition(pos);
	shape->node->SetScale(Vector3(0.5f, 0.5f, 0.5f));
	shape->node->UpdateNode(Node());
	Render::Instance()->drawSingle(GraphicsStorage::shaderIDs["geometry"], shape, *View**Projection, ShaderManager::Instance()->GetCurrentShaderID());
}

void DebugDraw::DrawLine(const Vector3& normal, const Vector3& position, float width)
{
	Matrix4 model = Matrix4::translation(position);
	double length = normal.vectLengt();
	Vector3 normalized = normal.vectNormalize();
	
	Vector3 axis = Vector3(0.f, 0.f, 1.f).crossProd(normalized);
	double tetha = acos(normalized.z);
	if (axis.squareMag() < 0.0001f)
	{
		axis = Vector3(1.f, 0.f, 0.f);
	}
	if (tetha != 0)
	{
		double  deg = (tetha * 180.f) / 3.14159f;
		model = Matrix4::rotateAngle(axis, deg)*model;
	}

	model = Matrix4::scale(Vector3(length, length, length))*model;
	
	GLuint prevShader = ShaderManager::Instance()->GetCurrentShaderID();
	GLuint wireframeShader = GraphicsStorage::shaderIDs["wireframe"];
	ShaderManager::Instance()->SetCurrentShader(wireframeShader);
	Line::Instance()->Draw(model, *View, *Projection, wireframeShader, width);
	ShaderManager::Instance()->SetCurrentShader(prevShader);
}


void DebugDraw::DrawNormal(const Vector3& normal, const Vector3& position, float width /*= 4.f*/)
{
	Matrix4 model = Matrix4::translation(position);
	Vector3 axis = Vector3(0.f, 0.f, 1.f).crossProd(normal);
	double tetha = acos(normal.z);
	if (axis.squareMag() < 0.0001f)
	{
		axis = Vector3(1.f, 0.f, 0.f);
	}
	if (tetha != 0)
	{
		double  deg = (tetha * 180.f) / 3.14159f;
		model = Matrix4::rotateAngle(axis, deg)*model;
	}
	GLuint prevShader = ShaderManager::Instance()->GetCurrentShaderID();
	GLuint wireframeShader = GraphicsStorage::shaderIDs["wireframe"];
	ShaderManager::Instance()->SetCurrentShader(wireframeShader);
	Line::Instance()->Draw(model, *View, *Projection, wireframeShader, width);
	Point::Instance()->Draw(model, *View, *Projection, wireframeShader);
	ShaderManager::Instance()->SetCurrentShader(prevShader);
}

void DebugDraw::DrawPlane(const Vector3& normal, const Vector3& position, const Vector3& halfExtent)
{
	Matrix4 model = Matrix4::translation(position);
	Vector3 axis = Vector3(0.f,0.f,1.f).crossProd(normal);
	double tetha = acos(normal.z);
	if (axis.squareMag() < 0.0001f)
	{
		axis = Vector3(1.f, 0.f, 0.f);
	}
	if (tetha != 0)
	{
		double  deg = (tetha * 180.f) / 3.14159f;
		model = Matrix4::rotateAngle(axis, deg)*model;
	}
	model = Matrix4::scale(halfExtent) * model;

	GLuint prevShader = ShaderManager::Instance()->GetCurrentShaderID();
	GLuint wireframeShader = GraphicsStorage::shaderIDs["wireframe"];
	ShaderManager::Instance()->SetCurrentShader(wireframeShader);
	Plane::Instance()->Draw(model, *View, *Projection, wireframeShader);
	ShaderManager::Instance()->SetCurrentShader(prevShader);
}


void DebugDraw::DrawPlaneN(const Vector3& normal, const Vector3& position, const Vector3& halfExtent /*= Vector3(1, 1, 1)*/)
{
	Matrix4 model = Matrix4::translation(position);
	Vector3 axis = Vector3(0.f, 0.f, 1.f).crossProd(normal);
	double tetha = acos(normal.z);
	if (axis.squareMag() < 0.0001f)
	{
		axis = Vector3(1.f, 0.f, 0.f);
	}
	if (tetha != 0)
	{
		double  deg = (tetha * 180.f) / 3.14159f;
		model = Matrix4::rotateAngle(axis, deg)*model;
	}
	model = Matrix4::scale(halfExtent) * model;

	GLuint prevShader = ShaderManager::Instance()->GetCurrentShaderID();
	GLuint wireframeShader = GraphicsStorage::shaderIDs["wireframe"];
	ShaderManager::Instance()->SetCurrentShader(wireframeShader);
	Plane::Instance()->Draw(model, *View, *Projection, wireframeShader);
	Line::Instance()->Draw(model, *View, *Projection, wireframeShader);
	Point::Instance()->Draw(model, *View, *Projection, wireframeShader);
	ShaderManager::Instance()->SetCurrentShader(prevShader);
}


void DebugDraw::DrawPoint(const Vector3& position, float size)
{
	GLuint prevShader = ShaderManager::Instance()->GetCurrentShaderID();
	GLuint wireframeShader = GraphicsStorage::shaderIDs["wireframe"];
	ShaderManager::Instance()->SetCurrentShader(wireframeShader);
	Point::Instance()->Draw(Matrix4::translation(position), *View, *Projection, wireframeShader, size);
	ShaderManager::Instance()->SetCurrentShader(prevShader);
}

void DebugDraw::DrawCrossHair(const Vector3F& color)
{
	double scale = 20.0;
	double offset = scale / 2.0;
	double windowWidth = (double)CameraManager::Instance()->GetCurrentCamera()->windowWidth;
	double windowHeight = (double)CameraManager::Instance()->GetCurrentCamera()->windowHeight;
	double x = windowWidth / 2.0;
	double y = windowHeight / 2.0;
	Matrix4 model1 = Matrix4::translation(x, y + offset, 0.0);
	Matrix4 model2 = Matrix4::translation(x - offset, y, 0.0);

	model1 = Matrix4::rotateAngle(Vector3(1.0, 0.0, 0.0), 90.0)*model1;
	model2 = Matrix4::rotateAngle(Vector3(0.0, 1.0, 0.0), 90.0)*model2;
	Matrix4 scaleM = Matrix4::scale(scale, scale, scale);
	model1 = scaleM * model1;
	model2 = scaleM * model2;
	Matrix4 view = Matrix4(1);
	Matrix4 proj = Matrix4::orthographicTopToBottom(-1.0, 2000.0, 0.0, windowWidth, windowHeight, 0.0);
	Line::Instance()->mat->color = color;

	GLuint prevShader = ShaderManager::Instance()->GetCurrentShaderID();
	GLuint wireframeShader = GraphicsStorage::shaderIDs["wireframe"];
	ShaderManager::Instance()->SetCurrentShader(wireframeShader);
	Line::Instance()->Draw(model1, view, proj, wireframeShader, 2.f);
	Line::Instance()->Draw(model2, view, proj, wireframeShader, 2.f);
	ShaderManager::Instance()->SetCurrentShader(prevShader);
}

void DebugDraw::DrawQuad()
{
	//bind vao before drawing
	Plane::Instance()->vao.Bind();

	// Draw the triangles !
	glDrawElements(GL_TRIANGLES, Plane::Instance()->vao.indicesCount, GL_UNSIGNED_SHORT, (void*)0); // mode, count, type, element array buffer offset
}

void DebugDraw::DrawRegion(int posX, int posY, int width, int height, const Texture* texture)
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(posX, posY, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	glViewport(posX, posY, width, height);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->handle);
	DrawQuad();

	glBindTexture(GL_TEXTURE_2D, 0);
	glViewport(0, 0, CameraManager::Instance()->GetCurrentCamera()->windowWidth, CameraManager::Instance()->GetCurrentCamera()->windowHeight);
}

void DebugDraw::Clear()
{
	lineSystems.clear();
	bbSystems.clear();
	pointSystems.clear();
}

void DebugDraw::Init(Object* debugObject)
{
	BoundingBoxSystem* bbs = new BoundingBoxSystem(3000);
	bbSystems.push_back(bbs);
	LineSystem* ls = new LineSystem(4000);
	lineSystems.push_back(ls);
	PointSystem* ps = new PointSystem(7000);
	pointSystems.push_back(ps);
	debugObject->AddComponent(bbs);
	debugObject->AddComponent(ls);
	debugObject->AddComponent(ps);
}

void
DebugDraw::DrawFastLineSystems(GLuint fboToDrawTo)
{
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, fboToDrawTo);

	GenerateFastLines();

	GLuint fastLineShader = GraphicsStorage::shaderIDs["fastLine"];
	ShaderManager::Instance()->SetCurrentShader(fastLineShader);
	for (auto& lSystem : lineSystems)
	{
		lSystem->Draw(CameraManager::Instance()->ViewProjection, fastLineShader, 1.f);
	}
}

void
DebugDraw::DrawFastPointSystems(GLuint fboToDrawTo)
{
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, fboToDrawTo);

	GLuint fastLineShader = GraphicsStorage::shaderIDs["fastLine"];
	ShaderManager::Instance()->SetCurrentShader(fastLineShader);
	for (auto& poSystem : pointSystems)
	{
		poSystem->Draw(CameraManager::Instance()->ViewProjection, fastLineShader, 5.f);
	}
}

void
DebugDraw::DrawBoundingBoxes(GLuint fboToDrawTo)
{
	GLuint fastBBShader = GraphicsStorage::shaderIDs["fastBB"];
	ShaderManager::Instance()->SetCurrentShader(fastBBShader);

	GenerateBoudingBoxes();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, fboToDrawTo);

	boundingBoxesDrawn = 0;
	for (auto& bbSystem : bbSystems)
	{
		boundingBoxesDrawn += bbSystem->Draw(CameraManager::Instance()->ViewProjection, fastBBShader);
	}	
}

void DebugDraw::GenerateBoudingBoxes()
{	
	for (auto& bbSystem : bbSystems)
	{
		for (auto obj : Scene::Instance()->allObjects)
		{
			if (obj->inFrustum)
			{
				FastBoundingBox* fastOBB = bbSystem->GetBoundingBoxOnce();
				fastOBB->color = &obj->bounds->obb.color;
				fastOBB->model = &obj->bounds->obb.model;

				FastBoundingBox* fastAABB = bbSystem->GetBoundingBoxOnce();
				fastAABB->color = &obj->bounds->aabb.color;
				fastAABB->model = &obj->bounds->aabb.model;
			}
		}
	}
}

void
DebugDraw::GenerateFastLines()
{
	if (DebugDraw::Instance()->lineSystems.empty()) return;
	for (auto& child : Scene::Instance()->SceneObject->node->children)
	{
		GenerateFastLineChildren(Scene::Instance()->SceneObject->node, child);
	}
}

void
DebugDraw::GenerateFastLineChildren(Node* parent, Node* child)
{
	FastLine* line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->AttachEndA(parent);
	line->AttachEndB(child);
	for (auto& childOfChild : child->children)
	{
		GenerateFastLineChildren(child, childOfChild);
	}
}