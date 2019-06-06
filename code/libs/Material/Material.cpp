#include "Material.h"
#include "Texture.h"
#include "Vao.h"

using namespace mwm;

Material::Material()
{

	color.vect[0] = 0.f;
	color.vect[1] = 0.f;
	color.vect[2] = 0.f;

	shininess = 40.0f;

	tileX = 1;
	tileY = 1;
}

Material::~Material()
{
	textures.clear();
}

void Material::ActivateAndBind() const
{
	for (size_t i = 0; i < textures.size(); i++)
	{
		textures[i]->ActivateAndBind(i);
	}
}

void Material::AddTexture(Texture *newTexture)
{
	textures.push_back(newTexture);
}

void Material::AssignTexture(Texture *newTexture, int textureSlot)
{
	if (textures.size() == 0) textures.push_back(newTexture);
	else if (textures.size() > textureSlot) textures[textureSlot] = newTexture;
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