#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "LightProperties.h"
#include <vector>
#include "BlurMode.h"

class Material;
class Texture;
class Object;
class FrameBuffer;

class SpotLight : public Component
{
public:
	SpotLight();
	~SpotLight();
	void Update();
	void Init(Object* parent);
	mwm::Matrix4 ProjectionMatrix; //used when drawing depth
	mwm::Vector3F LightInvDir; //used when drawing light 
	mwm::Matrix4 LightMatrixVP; //used when drawing depth
	mwm::Matrix4F BiasedLightMatrixVP; //used when drawing lights to sample depth
	void SetCutOff(float cutOffInDegrees);
	void SetOuterCutOff(float outerCutOffInDegrees);
	void SetRadius(float newRadius);
	FrameBuffer * GenerateShadowMapBuffer(int width = 256, int height = 256);
	void DeleteShadowMapBuffer();
	void GenerateBlurShadowMapBuffer(BlurMode mode = BlurMode::OneSize, int blurLevels = 4);
	void DeleteShadowMapBlurBuffer();
	void ResizeShadowMap(int width, int height);
	
	int activeBlurLevel = 3;
	int shadowBlurLevels = 4;
	float blurIntensity = 0.5f;
	float shadowFadeRange = 10.f;
	BlurMode blurMode = BlurMode::None;
	float innerCutOff = 12.5f;
	float outerCutOff = 17.5f;
	float cosInnerCutOff = 0;
	float cosOuterCutOff = 0;
	float outerCutOffClamped = 0; //we don't want the cut off to reach the edges of the cone
	float radius = 30.f;
	double near = 0.1;
	Attenuation attenuation;
	LightProperties properties;
	FrameBuffer* shadowMapBuffer;
	Texture* shadowMapTexture;
	FrameBuffer* pingPongBuffers[2];
	std::vector<FrameBuffer*> multiBlurBufferStart;
	std::vector<FrameBuffer*> multiBlurBufferTarget;

	bool shadowMapBlurActive = false;
	bool shadowMapActive = false;
	bool CanCastShadow();
	bool CanBlurShadowMap();
private:
	
};

