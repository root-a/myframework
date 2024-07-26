#include "DirectionalLight.h"
#include <algorithm>
#include "Object.h"
#include "CameraManager.h"
#include "Camera.h"
#include "Frustum.h"
#include "DebugDraw.h"
#include "PointSystem.h"
#include "LineSystem.h"
#include "BoundingBoxSystem.h"

DirectionalLight::DirectionalLight()
{
	glm::quat qXangle = glm::quat(glm::radians(108.0f), glm::vec3(1.0, 0.0, 0.0));
	glm::quat qYangle = glm::quat(glm::radians(162.0f), glm::vec3(0.0, 1.0, 0.0));
	glm::mat3 rotationMatrix = glm::mat3(qYangle * qXangle);
	glm::vec3 lightForward = MathUtils::GetForward(rotationMatrix);
	LightInvDir = -1.0f * lightForward;

	
	ProjectionMatrix = glm::ortho(radius, -radius, -radius, radius, -radius, radius);
	
	glm::mat4 lightViewMatrix = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) + lightForward, glm::vec3(0, 1, 0));
	LightMatrixVP = ProjectionMatrix * lightViewMatrix;
	BiasedLightMatrixVP = MathUtils::BiasMatrix() * LightMatrixVP;
}

DirectionalLight::~DirectionalLight()
{
}

void DirectionalLight::Update()
{
	//we should calculate this if the light is dynamic
	//otherwise set it only when changing parameters
	glm::mat3 rotationMatrix = object->node->GetWorldRotation3();
	glm::vec3 lightForward = MathUtils::GetForward(rotationMatrix);
	LightInvDir = -1.0f * lightForward;

	if (CanCastShadow())
	{
		Camera* camera = CameraManager::Instance()->GetCurrentCamera();
		camera->CalculateFrustumVertices();
#pragma region drawCamFrustum
		DebugDraw::Instance()->DrawFrustum(camera->frustumVertices);
#pragma endregion
#pragma region calculateShadowFrustum
		//dirLightShadow
		glm::mat4 lightViewMatrix = glm::lookAt(object->node->GetWorldPosition(), object->node->GetWorldPosition()+lightForward, glm::vec3(0, 1, 0));

		glm::vec3 currentVertex = glm::mat3(lightViewMatrix) * camera->frustumVertices[0];
		glm::vec3 max = currentVertex;
		glm::vec3 min = currentVertex;

		if (currentVertex.x > max.x) max.x = currentVertex.x;
		if (currentVertex.y > max.y) max.y = currentVertex.y;
		if (currentVertex.z > max.z) max.z = currentVertex.z;

		if (currentVertex.x < min.x) min.x = currentVertex.x;
		if (currentVertex.y < min.y) min.y = currentVertex.y;
		if (currentVertex.z < min.z) min.z = currentVertex.z;

		for (int i = 1; i < 8; ++i)
		{
			currentVertex = glm::mat3(lightViewMatrix) * camera->frustumVertices[i];
			if (currentVertex.x > max.x) max.x = currentVertex.x;
			if (currentVertex.y > max.y) max.y = currentVertex.y;
			if (currentVertex.z > max.z) max.z = currentVertex.z;

			if (currentVertex.x < min.x) min.x = currentVertex.x;
			if (currentVertex.y < min.y) min.y = currentVertex.y;
			if (currentVertex.z < min.z) min.z = currentVertex.z;
		}

		glm::vec4 center((min + max) * 0.5f, 1.0f);
		glm::mat4 lightViewMatrixInverse = glm::inverse(lightViewMatrix);
		glm::vec3 centerInWorld = (lightViewMatrixInverse * center);
#pragma endregion
#pragma region drawShadowFrustum
		//------------------------------------------------------------------
		FastPoint* point = DebugDraw::Instance()->pointSystems.front()->GetPointOnce();
		point->data.position = centerInWorld;
		point->data.color = glm::vec4(1, 1, 1, 1);

		FastBoundingBox* newBB = DebugDraw::Instance()->bbSystems.front()->GetBoundingBoxOnce();
		glm::mat4 model(1);
		glm::mat4 scale(1);
		glm::mat4 position(1);
		MathUtils::SetPosition(position, centerInWorld); //center in world space
		glm::vec3 color(0, 1, 1);
		MathUtils::SetScale(scale, max - min);
		model = position * object->node->GetWorldRotation() * scale;
		newBB->data.model = model;
		newBB->data.color = color;
		//------------------------------------------------------------------
		point = DebugDraw::Instance()->pointSystems.front()->GetPointOnce();
		point->data.position = center;
		point->data.color = glm::vec4(1, 1, 1, 1);

		newBB = DebugDraw::Instance()->bbSystems.front()->GetBoundingBoxOnce();
		glm::mat4 model2(1);
		MathUtils::SetScale(model2, max - min);
		MathUtils::SetPosition(model2, center);
		model2 = model2;
		glm::vec3 color2(0, 0, 1);
		newBB->data.model = model2;
		newBB->data.color = color2;

		for (int i = 0; i < 8; ++i) //frustum points
		{
			point = DebugDraw::Instance()->pointSystems.front()->GetPointOnce();
			point->data.position = camera->frustumVertices[i];
			point->data.color = glm::vec4(0, 1, 0, 1);
		}

		for (int i = 0; i < 8; ++i) //frustum points in light space
		{
			point = DebugDraw::Instance()->pointSystems.front()->GetPointOnce();
			point->data.position = glm::mat3(lightViewMatrix) * camera->frustumVertices[i];
			point->data.color = glm::vec4(1, 1, 0, 1);
		}
#pragma endregion		
		//ProjectionMatrix.setIdentity();
		//double width = max.x - min.x;
		//double height = max.y - min.y;
		//double length = max.z - min.z;
		//ProjectionMatrix[1][1] = width * 0.5;
		//ProjectionMatrix[2][2] = height * 0.5;
		//ProjectionMatrix[3][3] = length * -0.5;
		//ProjectionMatrix[4][4] = 1;
		float halfWidth = (max.x - min.x) * 0.5;
		float halfHeight = (max.y - min.y) * 0.5;
		float halfLength = (max.z - min.z) * 0.5;
		ProjectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, -halfLength, halfLength);
		//ProjectionMatrix = Matrix4::orthographic(-radius, radius, radius, -radius, radius, -radius);

 
		lightViewMatrix = glm::lookAt(centerInWorld + object->node->GetWorldPosition(), centerInWorld + object->node->GetWorldPosition() + lightForward, glm::vec3(0, 1, 0));
		//lightViewMatrix = Matrix4::lookAt(camera->GetPosition() + object->node->GetWorldPosition(), camera->GetPosition() + object->node->GetWorldPosition() + lightForward, Vector3(0, 1, 0));
		LightMatrixVP = ProjectionMatrix * lightViewMatrix;
		BiasedLightMatrixVP = MathUtils::BiasMatrix() * LightMatrixVP;
	}
}

Component* DirectionalLight::Clone()
{
	return new DirectionalLight(*this);
}

void DirectionalLight::SetProjectionRadius(double newRadius)
{
	radius = newRadius;
}

bool DirectionalLight::CanCastShadow()
{
	return shadowMapActive;
}

bool DirectionalLight::CanBlurShadowMap()
{
	return shadowMapBlurActive;
}