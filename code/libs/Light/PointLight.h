#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "LightProperties.h"
#include <vector>
#include "BlurMode.h"

class Material;
class Texture;
class FrameBuffer;
class Texture;

class PointLight : public Component
{
public:
	PointLight();
	~PointLight();
	void Update();
	Matrix4 ProjectionMatrix; //used when drawing depth
	float ProjectionSize = 150.f;
	int activeBlurLevel = 3;
	float blurIntensity = 0.5f;
	float shadowFadeRange = 10.f;
	BlurMode blurMode = BlurMode::None;
	Attenuation attenuation;
	LightProperties properties;
	void UpdateScale();
	FrameBuffer* GenerateShadowMapBuffer(int width, int height);
	void DeleteShadowMapBuffer();
	void ResizeShadowMap(int width, int height);
	FrameBuffer* shadowMapBuffer;
	Texture* shadowMapTexture;
	float near = 0.1f;
	//float far = 50;
	float fov = 90;
	std::vector<Matrix4> LightMatrixesVP; //used when drawing depth

	bool shadowMapBlurActive = true;
	bool shadowMapActive = false;
	bool CanCastShadow();
	bool CanBlurShadowMap();
private:
};

