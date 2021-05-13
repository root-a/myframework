#pragma once
#include "MyMathLib.h"
#include <vector>
class Texture;
class Shader;
class Vao;

class Material
{
public:
	union
	{
		Vector4F colorShininess;
		struct { Vector3F color; float shininess; };
	};
	//every texture should have tiling property in materials
	union
	{
		Vector2F tile;
		struct { float tileX, tileY; };
	};
	Material();
	~Material();
	std::vector<Texture*> textures;
	std::vector<Vao*> vaos;
	
	void ActivateAndBind() const;
	Vector3F emission;
	void AddTexture(Texture *newTexture);
	void AssignTexture(Texture *newTexture, int textureSlot = 0);
	void SetShininess(float s);
	void SetColor(float r, float g, float b);
	void SetColor(const Vector3F& colorC);
private:

};

