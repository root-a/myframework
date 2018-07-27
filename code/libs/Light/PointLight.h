#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "Attenuation.h"
#include <vector>

class Material;
class Mesh;
class Texture;
class FrameBuffer;
class Texture;

class PointLight : public Component
{
public:
	PointLight();
	~PointLight();
	void Update();
	mwm::Matrix4 ProjectionMatrix; //used when drawing depth
	float ProjectionSize = 150.f;
	bool hasShadowMap = false;
	bool blurShadowMap = true;
	int blurLevel = 3;
	float blurIntensity = 0.5f;
	float shadowFadeRange = 10.f;
	bool oneSizeBlur = true;
	Attenuation attenuation;
	void UpdateScale();
	void SetAttenuation(float constant, float linear, float exponential);
	void SetConstant(float constant);
	void SetLinear(float linear);
	void SetExponential(float exponential);
	FrameBuffer* GenerateShadowMapBuffer(int width, int height);
	void ResizeShadowMap(int width, int height);
	FrameBuffer* shadowMapBuffer;
	Texture* shadowMapTexture;
	float near = 0.1f;
	//float far = 50;
	float fov = 90;
	std::vector<mwm::Matrix4> LightMatrixesVP; //used when drawing depth
};

