#include "GraphicsStorage.h"
#include "Material.h"
#include "Mesh.h"
#include "Texture2D.h"
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

void GraphicsStorage::ClearOBJs()
{
	for (auto& object : objects)
	{
		delete object;
	}
	objects.clear();
}

std::map<std::string, Mesh*> GraphicsStorage::meshes;
std::vector<Texture2D*> GraphicsStorage::textures;
std::vector<Material*> GraphicsStorage::materials;
std::vector<OBJ*> GraphicsStorage::objects;