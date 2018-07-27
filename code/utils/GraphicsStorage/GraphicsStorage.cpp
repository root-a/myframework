#include "GraphicsStorage.h"
#include "Material.h"
#include "Mesh.h"
#include "Texture.h"
#include "OBJ.h"
#include <GL/glew.h>

GraphicsStorage::GraphicsStorage()
{
}

GraphicsStorage::~GraphicsStorage()
{
}

void GraphicsStorage::ClearMaterials()
{
	for (auto& material : materials)
	{
		delete material;
	}
	materials.clear();
}

void GraphicsStorage::ClearMeshes()
{
	for (auto& mesh : meshes)
	{
		delete mesh.second;
	}
	meshes.clear();
}

void GraphicsStorage::ClearTextures()
{
	for (auto& texture : textures)
	{
		delete texture;
	}
	textures.clear();
}

void GraphicsStorage::ClearCubemaps()
{
	for (auto& cubemap : cubemaps)
	{
		delete cubemap;
	}
	cubemaps.clear();
}

void GraphicsStorage::ClearOBJs()
{
	for (auto& object : objects)
	{
		delete object;
	}
	objects.clear();
}

std::unordered_map<std::string, Mesh*> GraphicsStorage::meshes;
std::vector<Texture*> GraphicsStorage::textures;
std::vector<Material*> GraphicsStorage::materials;
std::vector<OBJ*> GraphicsStorage::objects;
std::vector<Texture*> GraphicsStorage::cubemaps;