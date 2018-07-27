#include "DirectionalLight.h"
#include <algorithm>
#include "Object.h"
#include "FBOManager.h"
#include "FrameBuffer.h"

using namespace mwm;

DirectionalLight::DirectionalLight()
{
	Quaternion qXangle = Quaternion(108, Vector3(1.0, 0.0, 0.0));
	Quaternion qYangle = Quaternion(162, Vector3(0.0, 1.0, 0.0));
	Matrix3 rotationMatrix = (qYangle*qXangle).ConvertToMatrix3();
	Vector3 lightForward = rotationMatrix.getForward();
	LightInvDir = -1.0 * lightForward.toFloat();

	ProjectionMatrix = Matrix4::orthographic(-radius, radius, radius, -radius, radius, -radius);
}

DirectionalLight::~DirectionalLight()
{
}

void DirectionalLight::Update()
{
	Matrix3 rotationMatrix = object->GetWorldRotation3();
	Vector3 lightForward = rotationMatrix.getForward();
	LightInvDir = -1.0 * lightForward.toFloat();

	if (hasShadowMap)
	{
		ProjectionMatrix = Matrix4::orthographic(-radius, radius, radius, -radius, radius, -radius);
		Matrix4 lightViewMatrix = Matrix4::lookAt(object->GetWorldPosition(), object->GetWorldPosition() + lightForward, Vector3(0, 1, 0));
		LightMatrixVP = lightViewMatrix * ProjectionMatrix;
		BiasedLightMatrixVP = (LightMatrixVP*Matrix4::biasMatrix()).toFloat();
	}
}

void DirectionalLight::SetProjectionRadius(double newRadius)
{
	radius = newRadius;
	
}
