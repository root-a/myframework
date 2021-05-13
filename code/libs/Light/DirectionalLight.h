#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "BlurMode.h"
#include "LightProperties.h"

class Material;
class Texture;
class FrameBuffer;

class DirectionalLight : public Component
{
public:
	DirectionalLight();
	~DirectionalLight();
	void Update();
	Matrix4 ProjectionMatrix; //used when drawing depth
	Vector3F LightInvDir; //used when drawing light 
	//Matrix4 LightMatrixVP; //used when drawing depth
	Matrix4F LightMatrixVP; //used when drawing depth
	Matrix4F BiasedLightMatrixVP; //used when drawing lights to sample depth
	void SetProjectionRadius(double radius);
	float radius = 150.f;
	int activeBlurLevel = 3;
	float blurIntensity = 0.5f;
	float shadowFadeRange = 10.f;
	BlurMode blurMode = BlurMode::OneSize;
	LightProperties properties;
	bool shadowMapActive = false;
	bool shadowMapBlurActive = false;
	bool CanCastShadow();
	bool CanBlurShadowMap();
private:
};

