#include "GraphicsStorage.h"
#include "Material.h"
#include "Texture.h"
#include "OBJ.h"
#include "Vao.h"
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

void GraphicsStorage::ClearVaos()
{
	for (auto& vao : vaos)
	{
		delete vao.second;
	}
	vaos.clear();
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
	for (auto& obj : objs)
	{
		delete obj.second;
	}
	objs.clear();
}

void GraphicsStorage::ClearShaders()
{
	for (auto& shader : shaderIDs)
	{
		glDeleteProgram(shader.second);
	}
	shaderIDs.clear();
	shaderPaths.clear();
}

void GraphicsStorage::Clear()
{
	ClearVaos();
	ClearTextures();
	ClearCubemaps();
	ClearShaders();
	ClearOBJs();
	ClearMaterials();
}

std::unordered_map<std::string, Vao*> GraphicsStorage::vaos;
std::vector<Texture*> GraphicsStorage::textures;
std::vector<Material*> GraphicsStorage::materials;
std::unordered_map<std::string, OBJ*> GraphicsStorage::objs;
std::vector<Texture*> GraphicsStorage::cubemaps;
std::unordered_map<std::string, GLuint> GraphicsStorage::shaderIDs;
std::unordered_map<std::string, ShaderPaths> GraphicsStorage::shaderPaths;
