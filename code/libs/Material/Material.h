#pragma once
#include "MyMathLib.h"
#include <vector>
class Texture;

class Material
{
public:
	union
	{
		mwm::Vector4F properties;
		struct { float metallic, diffuseIntensity, specularIntensity, shininess; };
	};
	union
	{
		mwm::Vector2F tile;
		struct { float tileX, tileY; };
	};
	Material();
	~Material();
	std::vector<Texture*> textures;
	void ActivateAndBind() const;
	mwm::Vector3F emission;    // Ecm   
	//float metallic;     // Acm   
	//float diffuseIntensity;     // Dcm   diffuse sets is the diffuse intensity, currently diffuse color is obtained from texture
	mwm::Vector3F color;		 // color over texture if texture not preset used as shader diffuse color
	//float specularIntensity;    // Scm   
	//float shininess;  // Srm  
	void AddTexture(Texture *newTexture);
	void AssignTexture(Texture *newTexture, int textureSlot = 0);
	void SetDiffuseIntensity(float d);
	void SetSpecularIntensity(float spec);
	void SetShininess(float s);
	void SetMetallic(float m);
	void SetColor(float r, float g, float b);
	void SetColor(const mwm::Vector3F& colorC);
private:

};

