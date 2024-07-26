#pragma once
#include <vector>
#include "RenderElement.h"

class Texture;

struct TextureAndSlot
{
	Texture* texture;
	int slot;
	bool operator < (const TextureAndSlot& b) const
	{
		return (slot < b.slot);
	}
};


class TextureProfile : public RenderElement
{
public:
	TextureProfile();
	~TextureProfile();
	
	void SetUp();
	std::vector<TextureAndSlot> textures;
	void AddTexture(Texture *newTexture, int slot);
	void RemoveTextureAtSlot(Texture *newTexture, int slot);
	void RemoveTexture(Texture *newTexture);
	void AssignTexture(Texture *newTexture, int textureSlot);
	void SetTextureSlot(Texture *newTexture, int textureSlot);
	Texture* GetTexture(int slot);
	void ActivateAndBindTextures() const;
	void Execute();
	std::string path;
private:
};