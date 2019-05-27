#pragma once
#include "MyMathLib.h"
#include <vector>
class Texture;

class Material
{
public:
	union
	{
		mwm::Vector4F colorShininess;
		struct { mwm::Vector3F color; float shininess; };
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
	void AddTexture(Texture *newTexture);
	void AssignTexture(Texture *newTexture, int textureSlot = 0);
	void SetShininess(float s);
	void SetColor(float r, float g, float b);
	void SetColor(const mwm::Vector3F& colorC);
private:

};

