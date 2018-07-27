#pragma once
#include "MyMathLib.h"
#include "Component.h"

class Material;
class Mesh;
class Texture;
class FrameBuffer;

class DirectionalLight : public Component
{
public:
	DirectionalLight();
	~DirectionalLight();
	void Update();
	mwm::Matrix4 ProjectionMatrix; //used when drawing depth
	mwm::Vector3F LightInvDir; //used when drawing light 
	mwm::Matrix4 LightMatrixVP; //used when drawing depth
	mwm::Matrix4F BiasedLightMatrixVP; //used when drawing lights to sample depth
	void SetProjectionRadius(double radius);
	float radius = 150.f;
	bool blurShadowMap = true;
	int blurLevel = 3;
	float blurIntensity = 0.5f;
	float shadowFadeRange = 10.f;
	bool oneSizeBlur = true;
	bool hasShadowMap = false;
	
};

