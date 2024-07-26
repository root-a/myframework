#include "TextureProfile.h"
#include "Texture.h"

TextureProfile::TextureProfile()
{
}

TextureProfile::~TextureProfile()
{
}

void TextureProfile::SetUp()
{
}

void TextureProfile::ActivateAndBindTextures() const
{
	for (auto tas : textures)
	{
		tas.texture->ActivateAndBind(tas.slot);
	}
}

void TextureProfile::AddTexture(Texture * newTexture, int slot)
{
	TextureAndSlot tas = TextureAndSlot();
	tas.texture = newTexture;
	tas.slot = slot;
	textures.push_back(tas);
}

void TextureProfile::RemoveTextureAtSlot(Texture * newTexture, int slot)
{
	for (int i = 0; i < textures.size(); i++)
	{
		if (textures[i].texture == newTexture && textures[i].slot == slot)
		{
			textures[i] = textures.back();
			textures.pop_back();
			return;
		}
	}
}

void TextureProfile::RemoveTexture(Texture* newTexture)
{
	for (int i = 0; i < textures.size(); i++)
	{
		if (textures[i].texture == newTexture)
		{
			textures[i] = textures.back();
			textures.pop_back();
			return;
		}
	}
}

void TextureProfile::AssignTexture(Texture *newTexture, int textureSlot)
{
	//is this slot taken
	for (int i = 0; i < textures.size(); i++)
	{
		if (textures[i].slot == textureSlot)
		{
			textures[i] = textures.back();
			textures.pop_back();
			break;
		}
	}
	//add new texture
	AddTexture(newTexture, textureSlot);
}

void TextureProfile::SetTextureSlot(Texture* newTexture, int textureSlot)
{
	//is this slot taken
	for (int i = 0; i < textures.size(); i++)
	{
		if (textures[i].slot == textureSlot)
		{
			if (textures[i].texture == newTexture) return;
			textures[i] = textures.back();
			textures.pop_back();
			break;
		}
	}
	//change slot on existing texture
	for (auto& tas : textures)
	{
		if (tas.texture == newTexture)
		{
			tas.slot = textureSlot;
			return;
		}
	}
}

Texture* TextureProfile::GetTexture(int slot)
{
	for (auto& texSlot : textures)
	{
		if (texSlot.slot == slot)
		{
			return texSlot.texture;
		}
	}
	return nullptr;
}

void TextureProfile::Execute()
{
	ActivateAndBindTextures();
}
