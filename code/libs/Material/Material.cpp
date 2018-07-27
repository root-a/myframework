#include "Material.h"
#include "Texture.h"

using namespace mwm;

Material::Material()
{
	metallic = 1.f;

	specularIntensity = 0.25f;

	diffuseIntensity = 1.f;

	color.vect[0] = 0.f;
	color.vect[1] = 0.f;
	color.vect[2] = 0.f;

	shininess = 10.0f;

	tileX = 1;
	tileY = 1;
}

Material::~Material()
{
}

void Material::AssignTexture(Texture *newTexture)
{
	this->texture = newTexture;
}
void Material::SetDiffuseIntensity(float d)
{
	this->diffuseIntensity = d;
}

void Material::SetMetallic(float m)
{
	this->metallic = m;
}

void Material::SetSpecularIntensity(float spec)
{
	this->specularIntensity = spec;
}

void Material::SetColor(float r, float g, float b)
{
	this->color.vect[0] = r;
	this->color.vect[1] = g;
	this->color.vect[2] = b;
}

void Material::SetColor(const mwm::Vector3F& colorC)
{
	color = colorC;
}

void Material::SetShininess(float s)
{
	this->shininess = s;
}