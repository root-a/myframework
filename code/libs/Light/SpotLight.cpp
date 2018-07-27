#include "SpotLight.h"
#include <algorithm>
#include "Object.h"
#include "Material.h"
#include "Texture.h"
#include "FBOManager.h"
#include "FrameBuffer.h"

using namespace mwm;

SpotLight::SpotLight(Object* owner)
{
	object = owner;
	Quaternion qXangle = Quaternion(108, Vector3(1.0, 0.0, 0.0));
	Quaternion qYangle = Quaternion(162, Vector3(0.0, 1.0, 0.0));
	Matrix3 rotationMatrix = (qYangle*qXangle).ConvertToMatrix3();
	Vector3 lightForward = rotationMatrix.getForward();
	LightInvDir = -1.0 * lightForward.toFloat();

	SetCutOff(12.5f);
	SetOuterCutOff(17.5f);
	shadowMapBuffer = nullptr;
	castShadow = false;
}

SpotLight::~SpotLight()
{
}

void SpotLight::Update()
{
	Matrix3 rotationMatrix = object->GetWorldRotation3(); 
	Vector3 lightForward = rotationMatrix.getForward();
	LightInvDir = -1.0 * lightForward.toFloat();

	if (shadowMapBuffer != nullptr)
	{
		ProjectionMatrix = Matrix4::OpenGLPersp(MathUtils::ToDegrees((outerCutOffClamped) * 2.0), shadowMapTexture->aspect, near, radius*1.3);
		Matrix4 lightViewMatrix = Matrix4::lookAt(object->GetWorldPosition(), object->GetWorldPosition() + lightForward, Vector3(0, 1, 0));
		LightMatrixVP = lightViewMatrix * ProjectionMatrix;
		BiasedLightMatrixVP = (LightMatrixVP*Matrix4::biasMatrix()).toFloat();
	}
}

void SpotLight::SetCutOff(float cutOffInDegrees)
{
	float cutOffInRange = std::max(std::min(cutOffInDegrees, 88.f), 0.f);
	cutOffInRange = std::min(outerCutOffClamped, (float)MathUtils::ToRadians(cutOffInRange)); //smaller than outerCutOffClamped, cutOff is not used in scaling, only for shader, so mainly we don't want it to exceed the outerCutOffClamped
	innerCutOff = cutOffInRange; //this is not used anywhere we just store it for now
	cosInnerCutOff = std::cos(innerCutOff); //ready for shader
}

void SpotLight::SetOuterCutOff(float outerCutOffInDegrees) //5
{
	float outerCutOffInRange = std::max(std::min(outerCutOffInDegrees+5.f, 88.f), 0.f); //smaller than 90 because we don't want to break tan calculation and bigger than 0, only positive angles
	outerCutOff = MathUtils::ToRadians(outerCutOffInRange); //used for scaling
	outerCutOffClamped = (float)MathUtils::ToRadians(outerCutOffInRange - 5.0); //we don't want the cut off to reach cone edges, needed for limiting the inner cutOff
	cosOuterCutOff = std::cos(outerCutOffClamped); //ready for shader
	SetCutOff(MathUtils::ToDegrees(innerCutOff)); //update cutoff since it's dependent on outerCutOff, we don't want cutOff to be larger than outerCutOff, or maybe we should allow it?

	float xyScale = tan(outerCutOff) * radius;
	object->SetScale(Vector3(xyScale, xyScale, radius));
}

void SpotLight::SetRadius(float newRadius)
{
	radius = newRadius;
	float xyScale = tan(outerCutOff) * radius;
	object->SetScale(Vector3(xyScale, xyScale, radius));
}

FrameBuffer * SpotLight::GenerateShadowMapBuffer(int width, int height)
{
	shadowMapBuffer = FBOManager::Instance()->Generate2DShadowMapBuffer(width,height);
	shadowMapTexture = shadowMapBuffer->textures[0];
	castShadow = true;
	return shadowMapBuffer;
}

void SpotLight::ResizeShadowMap(int width, int height)
{
	shadowMapBuffer->UpdateTextures(width, height);
}

void SpotLight::SetAttenuation(float constant, float linear, float exponential)
{
	attenuation.Constant = constant;
	attenuation.Linear = linear;
	attenuation.Exponential = exponential;
}

void SpotLight::SetConstant(float constant)
{
	attenuation.Constant = constant;
}

void SpotLight::SetLinear(float linear)
{
	attenuation.Linear = linear;
}

void SpotLight::SetExponential(float exponential)
{
	attenuation.Exponential = exponential;
}