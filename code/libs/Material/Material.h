#pragma once
#include "MyMathLib.h"

class Texture;

class Material
{
public:
	Material();
	~Material();
	Texture* texture;
	mwm::Vector3F emission;    // Ecm   
	float metallic;     // Acm   
	float diffuseIntensity;     // Dcm   diffuse sets is the diffuse intensity, currently diffuse color is obtained from texture
	mwm::Vector3F color;		 // color over texture if texture not preset used as shader diffuse color
	float specularIntensity;    // Scm   
	float shininess;  // Srm  
	void AssignTexture(Texture *newTexture);
	void SetDiffuseIntensity(float d);
	void SetSpecularIntensity(float spec);
	void SetShininess(float s);
	void SetMetallic(float m);
	void SetColor(float r, float g, float b);
	void SetColor(const mwm::Vector3F& colorC);
	float tileX;
	float tileY;
private:

};

