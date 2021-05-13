#include "DirectionalLight.h"
#include <algorithm>
#include "Object.h"



DirectionalLight::DirectionalLight()
{
	Quaternion qXangle = Quaternion(108, Vector3(1.0, 0.0, 0.0));
	Quaternion qYangle = Quaternion(162, Vector3(0.0, 1.0, 0.0));
	Matrix3 rotationMatrix = (qYangle*qXangle).ConvertToMatrix3();
	Vector3 lightForward = rotationMatrix.getForward();
	LightInvDir = -1.0 * lightForward.toFloat();

	ProjectionMatrix = Matrix4::orthographic(-radius, radius, radius, -radius, radius, -radius);

	Matrix4 lightViewMatrix = Matrix4::lookAt(Vector3(0, 0, 0), Vector3(0, 0, 0) + lightForward, Vector3(0, 1, 0));
	LightMatrixVP = lightViewMatrix * ProjectionMatrix;
	BiasedLightMatrixVP = (LightMatrixVP*Matrix4::biasMatrix().toFloat());
}

DirectionalLight::~DirectionalLight()
{
}

void DirectionalLight::Update()
{
	//we should calculate this if the light is dynamic
	//otherwise set it only when changing parameters
	Matrix3 rotationMatrix = object->node->GetWorldRotation3();
	Vector3 lightForward = rotationMatrix.getForward();
	LightInvDir = -1.0 * lightForward.toFloat();

	if (CanCastShadow())
	{
		ProjectionMatrix = Matrix4::orthographic(-radius, radius, radius, -radius, radius, -radius);
		Matrix4 lightViewMatrix = Matrix4::lookAt(object->node->GetWorldPosition(), object->node->GetWorldPosition() + lightForward, Vector3(0, 1, 0));
		LightMatrixVP = lightViewMatrix * ProjectionMatrix;
		BiasedLightMatrixVP = (LightMatrixVP*Matrix4::biasMatrix().toFloat());
	}
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