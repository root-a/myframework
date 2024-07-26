#pragma once
#include <GL/glew.h>
#include "DebugDraw.h"
#include "Object.h"
#include "Node.h"
#include "GraphicsStorage.h"
#include "Material.h"
#include "TextureProfile.h"
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
#include "SceneGraph.h"
#include "RigidBody.h"
#include "FBOManager.h"
#include "OBJ.h"
#include "BoundingBox.h"
#include "Line.h"
#include "Plane.h"
#include "Point.h"
#include "Box.h"
#include "ParticleSystem.h"
#include "CircleSystem.h"


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
	//debugMat = new Material();
	//debugShapes;
}

void DebugDraw::DrawShapeAtPos(const char* shapeName, const glm::vec3& pos)
{
	Object* shape = DebugDraw::Instance()->debugShapes[shapeName];
	shape->node->SetPosition(pos);
	shape->node->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));
	shape->node->UpdateNode(Node());
	Render::Instance()->drawSingle(GraphicsStorage::shaderIDs["GeometryPicking"], shape, *View**Projection, ShaderManager::Instance()->GetCurrentShaderID());
}

void DebugDraw::DrawLine(const glm::vec3& normal, const glm::vec3& position)
{
	glm::mat4 model = glm::translate(glm::mat4(1), position);
	float length = glm::length(normal);
	glm::vec3 normalized = glm::normalize(normal);
	
	glm::vec3 axis = glm::cross(glm::vec3(0.f, 0.f, 1.f), normalized);
	float tetha = acos(normalized.z);
	if (glm::dot(axis, axis) < 0.0001f)
	{
		axis = glm::vec3(1.f, 0.f, 0.f);
	}
	if (tetha != 0)
	{
		//float  deg = (tetha * 180.f) / 3.14159f;
		model = glm::rotate(model, tetha, axis);
	}

	model = glm::scale(model, glm::vec3(length, length, length));
	
	wireframeShader->Execute();
	Line::Instance()->Draw(model, *View, *Projection, wireframeShader->shaderID);
}


void DebugDraw::DrawNormal(const glm::vec3& normal, const glm::vec3& position)
{
	glm::mat4 model = glm::translate(glm::mat4(1), position);
	glm::vec3 axis = glm::cross(glm::vec3(0.f, 0.f, 1.f), normal);
	float tetha = acos(normal.z);
	if (glm::dot(axis, axis) < 0.0001f)
	{
		axis = glm::vec3(1.f, 0.f, 0.f);
	}
	if (tetha != 0)
	{
		//float  deg = (tetha * 180.f) / 3.14159f;
		model = glm::rotate(model, tetha, axis);
	}

	wireframeShader->Execute();
	Line::Instance()->Draw(model, *View, *Projection, wireframeShader->shaderID);
	Point::Instance()->Draw(model, *View, *Projection, wireframeShader->shaderID);
}

void DebugDraw::DrawFrustum(glm::vec3* frustumVertices) const
{
	FastPoint* point = DebugDraw::Instance()->pointSystems.front()->GetPointOnce();
	point->data.position = frustumVertices[(int)Camera::FrustumVertices::frustumCenter];
	point->data.color = glm::vec4(0, 1, 1, 1);

	FastLine* line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::frustumOrigin]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::nearUpperLeftCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);

	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::frustumOrigin]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::nearUpperRightCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);

	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::frustumOrigin]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::nearLowerLeftCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);

	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::frustumOrigin]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::nearLowerRightCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);

	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::nearUpperLeftCorner]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::farUpperLeftCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);

	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::nearUpperRightCorner]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::farUpperRightCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);

	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::nearLowerLeftCorner]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::farLowerLeftCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);

	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::nearLowerRightCorner]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::farLowerRightCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);

	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::nearUpperLeftCorner]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::nearUpperRightCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);

	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::nearLowerLeftCorner]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::nearLowerRightCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);

	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::nearLowerLeftCorner]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::nearUpperLeftCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);

	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::nearLowerRightCorner]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::nearUpperRightCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);

	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::farUpperLeftCorner]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::farUpperRightCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);

	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::farLowerLeftCorner]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::farLowerRightCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);
	
	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::farLowerLeftCorner]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::farUpperLeftCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);

	line = DebugDraw::Instance()->lineSystems.front()->GetLineOnce();
	line->SetPositionA(frustumVertices[(int)Camera::FrustumVertices::farLowerRightCorner]);
	line->SetPositionB(frustumVertices[(int)Camera::FrustumVertices::farUpperRightCorner]);
	line->data.colorA = glm::vec4(1, 0, 0, 1);
	line->data.colorB = glm::vec4(1, 0, 0, 1);
}

void DebugDraw::DrawPlane(const glm::vec3& normal, const glm::vec3& position, const glm::vec3& halfExtent)
{
	glm::mat4 model = glm::translate(glm::mat4(1), position);
	glm::vec3 axis = glm::cross(glm::vec3(0.f,0.f,1.f), normal);
	float tetha = acos(normal.z);
	if (glm::dot(axis, axis) < 0.0001f)
	{
		axis = glm::vec3(1.f, 0.f, 0.f);
	}
	if (tetha != 0)
	{
		//float  deg = (tetha * 180.f) / 3.14159f;
		model = glm::rotate(model, tetha, axis);
	}
	model = glm::scale(model, halfExtent);

	wireframeShader->Execute();
	Plane::Instance()->Draw(model, *View, *Projection, wireframeShader->shaderID);
}


void DebugDraw::DrawPlaneN(const glm::vec3& normal, const glm::vec3& position, const glm::vec3& halfExtent /*= glm::vec3(1, 1, 1)*/)
{
	glm::mat4 model = glm::translate(glm::mat4(1), position);
	glm::vec3 axis = glm::cross(glm::vec3(0.f, 0.f, 1.f), normal);
	float tetha = acos(normal.z);
	if (glm::dot(axis, axis) < 0.0001f)
	{
		axis = glm::vec3(1.f, 0.f, 0.f);
	}
	if (tetha != 0)
	{
		//float  deg = (tetha * 180.f) / 3.14159f;
		model = glm::rotate(model, tetha, axis);
	}
	model = glm::scale(model, halfExtent);

	wireframeShader->Execute();
	Plane::Instance()->Draw(model, *View, *Projection, wireframeShader->shaderID);
	Line::Instance()->Draw(model, *View, *Projection, wireframeShader->shaderID);
	Point::Instance()->Draw(model, *View, *Projection, wireframeShader->shaderID);
}


void DebugDraw::DrawPoint(const glm::vec3& position, float size)
{
	wireframeShader->Execute();
	Point::Instance()->Draw(glm::translate(glm::mat4(1), position), *View, *Projection, wireframeShader->shaderID, size);
}

void DebugDraw::DrawCrossHair(const glm::vec3& color)
{
	double scale = 20.0;
	double offset = scale / 2.0;
	double windowWidth = (double)CameraManager::Instance()->GetCurrentCamera()->windowWidth;
	double windowHeight = (double)CameraManager::Instance()->GetCurrentCamera()->windowHeight;
	double x = windowWidth / 2.0;
	double y = windowHeight / 2.0;
	glm::mat4 model1 = glm::translate(glm::mat4(1), glm::vec3(x, y + offset, 0.0));
	glm::mat4 model2 = glm::translate(glm::mat4(1), glm::vec3(x - offset, y, 0.0));

	model1 = glm::rotate(model1, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
	model2 = glm::rotate(model2, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
	model1 = glm::scale(model1, glm::vec3(scale, scale, scale));
	model2 = glm::scale(model2, glm::vec3(scale, scale, scale));
	glm::mat4 view = glm::mat4(1);
	glm::mat4 proj = glm::ortho(0.0, windowWidth, 0.0, windowHeight, -1.0, 2000.0);
	//glm::mat4 proj = glm::mat4::orthographicTopToBottom(-1.0, 2000.0, 0.0, windowWidth, windowHeight, 0.0);
	
	//Line::Instance()->mat->color = color;

	wireframeShader->Execute();
	Line::Instance()->Draw(model1, view, proj, wireframeShader->shaderID);
	Line::Instance()->Draw(model2, view, proj, wireframeShader->shaderID);
}

void DebugDraw::DrawQuad()
{
	//bind vao before drawing
	Plane::Instance()->vao.Bind();

	// Draw the triangles !
	Plane::Instance()->vao.Draw();
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
	BoundingBoxSystem* bbs = GraphicsStorage::assetRegistry.AllocAsset<BoundingBoxSystem>(3000);
	bbSystems.push_back(bbs);
	LineSystem* ls = GraphicsStorage::assetRegistry.AllocAsset<LineSystem>(4000);
	lineSystems.push_back(ls);
	PointSystem* ps = GraphicsStorage::assetRegistry.AllocAsset<PointSystem>(7000);
	pointSystems.push_back(ps);
	CircleSystem* cs = GraphicsStorage::assetRegistry.AllocAsset<CircleSystem>(7000);
	circleSystems.push_back(cs);
	debugObject->AddComponent(bbs);
	debugObject->AddComponent(ls);
	debugObject->AddComponent(ps);
	debugObject->AddComponent(cs);
	debugObject->AddComponent(GraphicsStorage::assetRegistry.AllocAsset<Node>());
	debugObject->name = "debug draw object";

	for (auto& shader : *GraphicsStorage::assetRegistry.GetPool<Shader>())
	{
		if (shader.name.compare("BB") == 0)
		{
			wireframeShader = &shader;
		}

		if (shader.name.compare("FastBB") == 0)
		{
			fastBBShader = &shader;
		}

		if (shader.name.compare("FastLine") == 0)
		{
			fastLineShader = &shader;
		}

		if (shader.name.compare("FastCircle") == 0)
		{
			fastCircleShader = &shader;
		}
	}
}

void
DebugDraw::DrawFastLineSystems(GLuint fboToDrawTo)
{
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, fboToDrawTo);

	//GenerateFastLines();

	fastLineShader->Execute();
	for (auto& lSystem : lineSystems)
	{
		lSystem->Draw(CameraManager::Instance()->ViewProjection, fastLineShader->shaderID);
	}
}

void DebugDraw::DrawFastCircleSystems(GLuint fboToDrawTo)
{
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, fboToDrawTo);

	fastCircleShader->Execute();
	for (auto& cSystem : circleSystems)
	{
		cSystem->Draw(CameraManager::Instance()->ViewProjection, fastCircleShader->shaderID);
	}
}

void
DebugDraw::DrawFastPointSystems(GLuint fboToDrawTo)
{
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, fboToDrawTo);

	fastLineShader->Execute();
	for (auto& poSystem : pointSystems)
	{
		poSystem->Draw(CameraManager::Instance()->ViewProjection, fastLineShader->shaderID, 5.f);
	}
}

void
DebugDraw::DrawBoundingBoxes(GLuint fboToDrawTo)
{
	fastBBShader->Execute();

	GenerateBoudingBoxes();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, fboToDrawTo);

	boundingBoxesDrawn = 0;
	for (auto& bbSystem : bbSystems)
	{
		boundingBoxesDrawn += bbSystem->Draw(CameraManager::Instance()->ViewProjection, fastBBShader->shaderID);
	}	
}

void DebugDraw::GenerateBoudingBoxes()
{	
	for (auto& bbSystem : bbSystems)
	{
		for (auto obj : SceneGraph::Instance()->objectsInFrustum)
		{
			if (obj->bounds != nullptr)
			{
				FastBoundingBox* fastOBB = bbSystem->GetBoundingBoxOnce();
				fastOBB->data.color = obj->bounds->obb.color;
				fastOBB->data.model = obj->bounds->obb.model;

				FastBoundingBox* fastAABB = bbSystem->GetBoundingBoxOnce();
				fastAABB->data.color = obj->bounds->aabb.color;
				fastAABB->data.model = obj->bounds->aabb.model;
			}
		}
	}
}

void
DebugDraw::GenerateFastLines()
{
	if (DebugDraw::Instance()->lineSystems.empty()) return;
	for (auto& child : SceneGraph::Instance()->SceneRoot.children)
	{
		GenerateFastLineChildren(&SceneGraph::Instance()->SceneRoot, child);
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