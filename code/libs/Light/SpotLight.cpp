#include "SpotLight.h"
#include <algorithm>
#include "Object.h"
#include "Material.h"
#include "Texture.h"
#include "FBOManager.h"
#include "FrameBuffer.h"
#include "GraphicsStorage.h"


SpotLight::SpotLight()
{
	glm::quat qXangle = glm::quat(glm::radians(108.0f), glm::vec3(1.0, 0.0, 0.0));
	glm::quat qYangle = glm::quat(glm::radians(162.0f), glm::vec3(0.0, 1.0, 0.0));
	glm::mat3 rotationMatrix = glm::mat3(glm::normalize(qYangle * qXangle));
	glm::vec3 lightForward = MathUtils::GetForward(rotationMatrix);
	LightInvDir = -1.0f * lightForward;
	shadowMapBuffer = nullptr;
	pingPongBuffers[0] = nullptr;
	pingPongBuffers[1] = nullptr;
	SetCutOff(12.5f);
}

SpotLight::~SpotLight()
{
}

void SpotLight::Update()
{
	glm::mat3 rotationMatrix = object->node->GetWorldRotation3();
	glm::vec3 lightForward = MathUtils::GetForward(rotationMatrix);
	LightInvDir = -1.0f * lightForward;

	if (shadowMapBuffer != nullptr)
	{
		ProjectionMatrix = glm::perspective(outerCutOffClamped * 2.0, shadowMapTexture->aspect, near, radius*1.3);
		glm::mat4 lightViewMatrix = glm::lookAt(object->node->GetWorldPosition(), object->node->GetWorldPosition() + lightForward, glm::vec3(0, 1, 0));
		LightMatrixVP = ProjectionMatrix * lightViewMatrix;
		BiasedLightMatrixVP = MathUtils::BiasMatrix() * LightMatrixVP;
	}
}

void SpotLight::Init(Object* parent)
{
	Component::Init(parent);
	SetOuterCutOff(17.5f);
}

void SpotLight::SetCutOff(float cutOffInDegrees)
{
	float cutOffInRange = std::max(std::min(cutOffInDegrees, 88.f), 0.f);
	cutOffInRange = std::min(outerCutOffClamped, (float)glm::radians(cutOffInRange)); //smaller than outerCutOffClamped, cutOff is not used in scaling, only for shader, so mainly we don't want it to exceed the outerCutOffClamped
	innerCutOff = cutOffInRange; //this is not used anywhere we just store it for now
	cosInnerCutOff = std::cos(innerCutOff); //ready for shader
}

void SpotLight::SetOuterCutOff(float outerCutOffInDegrees) //5
{
	float outerCutOffInRange = std::max(std::min(outerCutOffInDegrees+5.f, 88.f), 0.f); //smaller than 90 because we don't want to break tan calculation and bigger than 0, only positive angles
	outerCutOff = glm::radians(outerCutOffInRange); //used for scaling
	outerCutOffClamped = (float)glm::radians(outerCutOffInRange - 5.0); //we don't want the cut off to reach cone edges, needed for limiting the inner cutOff
	cosOuterCutOff = std::cos(outerCutOffClamped); //ready for shader
	SetCutOff(glm::degrees(innerCutOff)); //update cutoff since it's dependent on outerCutOff, we don't want cutOff to be larger than outerCutOff, or maybe we should allow it?

	float xyScale = tan(outerCutOff) * radius;
	object->node->SetScale(glm::vec3(xyScale, xyScale, radius));
}

void SpotLight::SetRadius(float newRadius)
{
	radius = newRadius;
	float xyScale = tan(outerCutOff) * radius;
	object->node->SetScale(glm::vec3(xyScale, xyScale, radius));
}

FrameBuffer * SpotLight::GenerateShadowMapBuffer(int width, int height)
{
	if (shadowMapBuffer == nullptr)
	{
		shadowMapBuffer = GraphicsStorage::assetRegistry.AllocAsset<FrameBuffer>((unsigned int)GL_FRAMEBUFFER);
		FBOManager::Instance()->AddFrameBuffer(shadowMapBuffer, false);
		shadowMapBuffer = FBOManager::Instance()->Generate2DShadowMapBuffer(shadowMapBuffer, width, height);
		shadowMapTexture = shadowMapBuffer->textures[0];
	}
	return shadowMapBuffer;
}

void SpotLight::DeleteShadowMapBuffer()
{
	if (shadowMapBuffer != nullptr)
	{
		shadowMapBuffer->DeleteAllTextures();
		FBOManager::Instance()->DeleteFrameBuffer(shadowMapBuffer);
		shadowMapBuffer = nullptr;
	}
}

void SpotLight::GenerateBlurShadowMapBuffer(BlurMode mode, int blurLevels)
{
	if (blurMode == BlurMode::None)
	{
		blurMode = mode;
		shadowBlurLevels = blurLevels;
		int width = shadowMapTexture->width;
		int height = shadowMapTexture->height;
		switch (blurMode)
		{
		case BlurMode::None:
			break;
		case BlurMode::OneSize:
			for (int i = 0; i < 2; i++)
			{
				FrameBuffer* pingPongBuffer = GraphicsStorage::assetRegistry.AllocAsset<FrameBuffer>((unsigned int)GL_FRAMEBUFFER);
				FBOManager::Instance()->AddFrameBuffer(pingPongBuffer, false);
				Texture* blurTexture = new Texture(GL_TEXTURE_2D, 0, GL_RG32F, width, height, GL_RG, GL_FLOAT, NULL, GL_COLOR_ATTACHMENT0);
				blurTexture->GenerateBindSpecify();
				blurTexture->SetLinear();
				blurTexture->SetClampingToBorder(Vector4F(1.f, 1.f, 1.f, 1.f));

				pingPongBuffer->RegisterTexture(blurTexture);

				pingPongBuffer->SpecifyTextures();

				pingPongBuffer->CheckAndCleanup();

				pingPongBuffers[i] = pingPongBuffer;
			}
			break;
		case BlurMode::MultiSize:
		{
			std::vector<FrameBuffer*>* bufferStorage = &multiBlurBufferStart;
			for (int i = 0; i < 2; i++)
			{
				FrameBuffer* multiBlurBuffer = GraphicsStorage::assetRegistry.AllocAsset<FrameBuffer>((unsigned int)GL_FRAMEBUFFER);
				FBOManager::Instance()->AddFrameBuffer(multiBlurBuffer, true);
				//multiBlurBuffer->dynamicSize = false;
				Texture* blurTexture = new Texture(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, GL_RGB, GL_FLOAT, NULL, GL_COLOR_ATTACHMENT0);
				blurTexture->GenerateBindSpecify();
				blurTexture->SetLinear();
				blurTexture->SetClampingToBorder(Vector4F(1.f, 1.f, 1.f, 1.f));

				multiBlurBuffer->RegisterTexture(blurTexture);

				multiBlurBuffer->SpecifyTextures();

				multiBlurBuffer->CheckAndCleanup();

				bufferStorage->push_back(multiBlurBuffer);
				FrameBuffer* parentBuffer = multiBlurBuffer;
				for (int j = 1; j < blurLevels; j++)
				{
					FrameBuffer* childBlurBuffer = GraphicsStorage::assetRegistry.AllocAsset<FrameBuffer>((unsigned int)GL_FRAMEBUFFER);
					FBOManager::Instance()->AddFrameBuffer(childBlurBuffer, false);
					Texture* blurTexture = new Texture(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, GL_RGB, GL_FLOAT, NULL, GL_COLOR_ATTACHMENT0);
					blurTexture->GenerateBindSpecify();
					blurTexture->SetLinear();
					blurTexture->SetClampingToBorder(Vector4F(1.f, 1.f, 1.f, 1.f));

					childBlurBuffer->RegisterTexture(blurTexture);

					childBlurBuffer->SpecifyTextures();

					childBlurBuffer->CheckAndCleanup();

					bufferStorage->push_back(childBlurBuffer);

					parentBuffer->RegisterChildBuffer(childBlurBuffer);
					parentBuffer = childBlurBuffer;
				}
				multiBlurBuffer->UpdateTextures(width, height);
				bufferStorage = &multiBlurBufferTarget;
			}
		}
			break;
		default:
			break;
		}
	}
}

void SpotLight::DeleteShadowMapBlurBuffer()
{
	switch (blurMode)
	{
	case BlurMode::None:
		break;
	case BlurMode::OneSize:
		for (size_t i = 0; i < 2; i++)
		{
			FrameBuffer* blurBuffer = pingPongBuffers[i];
			blurBuffer->DeleteAllTextures();
			FBOManager::Instance()->DeleteFrameBuffer(blurBuffer);
			pingPongBuffers[i] = nullptr;
		}
		break;
	case BlurMode::MultiSize:
		for (auto buffer : multiBlurBufferStart)
		{
			buffer->DeleteAllTextures();
			FBOManager::Instance()->DeleteFrameBuffer(buffer);
		}
		multiBlurBufferStart.clear();
		for (auto buffer : multiBlurBufferTarget)
		{
			buffer->DeleteAllTextures();
			FBOManager::Instance()->DeleteFrameBuffer(buffer);
		}
		multiBlurBufferTarget.clear();
		break;
	default:
		break;
	}
	blurMode = BlurMode::None;
}

void SpotLight::ResizeShadowMap(int width, int height)
{
	shadowMapBuffer->UpdateTextures(width, height);
	
	switch (blurMode)
	{
	case BlurMode::None:
		break;
	case BlurMode::OneSize:
		pingPongBuffers[0]->UpdateTextures(width, height);
		pingPongBuffers[1]->UpdateTextures(width, height);
		break;
	case BlurMode::MultiSize:
		multiBlurBufferStart[0]->UpdateTextures(width, height);
		multiBlurBufferTarget[0]->UpdateTextures(width, height);
		break;
	default:
		break;
	}
}

Component* SpotLight::Clone()
{
	return new SpotLight(*this);
}

bool SpotLight::CanCastShadow()
{
	return shadowMapBuffer != nullptr && shadowMapActive;
}

bool SpotLight::CanBlurShadowMap()
{
	return blurMode != BlurMode::None && shadowMapBlurActive;
}