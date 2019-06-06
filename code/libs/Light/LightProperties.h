#pragma once
struct Attenuation
{
	float Constant = 1.f;
	float Linear = 0.1f;
	float Exponential = 0.03f;
};

struct LightProperties
{
	mwm::Vector3F color = mwm::Vector3F(1.f,1.f,1.f);
	float power = 1.f;
	float ambient = 0.2f;
	float diffuse = 1.f;
	float specular = 0.5f;
};