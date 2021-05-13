#include "GraphicsStorage.h"
#include "ShaderBlock.h"
#include "Material.h"
#include "Texture.h"
#include "RenderBuffer.h"
#include "OBJ.h"
#include "Vao.h"
#include <GL/glew.h>
#include "DataRegistry.h"
#include "ShaderBlockData.h"

GraphicsStorage::GraphicsStorage()
{
}

GraphicsStorage::~GraphicsStorage()
{
}

void GraphicsStorage::ClearMaterials()
{
	for (auto material : materials)
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
		delete texture.second;
	}
	textures.clear();
}

void GraphicsStorage::ClearUniformBuffers()
{
	uniformBuffers.clear();
}

void GraphicsStorage::ClearUniformBuffersDatas()
{
	uniformBuffersDatas.clear();
}

void GraphicsStorage::ClearCubemaps()
{
	for (auto& cubemap : cubemaps)
	{
		delete cubemap.second;
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
	for (auto& shader : shaders)
	{
		delete shader.second;
	}
	shaderIDs.clear();
	shaderPaths.clear();
	shaders.clear();
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

ShaderBlock* GraphicsStorage::GetUniformBuffer(int index)
{
	for (auto buffer : uniformBuffers)
	{
		if (buffer->index == index)
		{
			return buffer;
		}
	}
	return nullptr;
}

ShaderBlockData* GraphicsStorage::GetUniformBufferData(int index)
{
	for (auto buffer : uniformBuffersDatas)
	{
		if (buffer->shaderBlock->index == index)
		{
			return buffer;
		}
	}
	return nullptr;
}

ShaderBlockData* GraphicsStorage::GetShaderStorageData(int index)
{
	for (auto buffer : shaderStoragesDatas)
	{
		if (buffer->shaderBlock->index == index)
		{
			return buffer;
		}
	}
	return nullptr;
}

ShaderBlock* GraphicsStorage::GetShaderStorageBuffer(int index)
{
	for (auto buffer : shaderStorageBuffers)
	{
		if (buffer->index == index)
		{
			return buffer;
		}
	}
	return nullptr;
}

ShaderBlock* GraphicsStorage::GetUniformBuffer(const char * name)
{
	for (auto buffer : uniformBuffers)
	{
		if (buffer->name.compare(name) == 0)
		{
			return buffer;
		}
	}
	return nullptr;
}

ShaderBlockData* GraphicsStorage::GetUniformBufferData(const char * name)
{
	for (auto buffer : uniformBuffersDatas)
	{
		if (buffer->shaderBlock->name.compare(name) == 0)
		{
			return buffer;
		}
	}
	return nullptr;
}

ShaderBlockData* GraphicsStorage::GetShaderStorageBufferData(const char * name)
{
	for (auto buffer : shaderStoragesDatas)
	{
		if (buffer->shaderBlock->name.compare(name) == 0)
		{
			return buffer;
		}
	}
	return nullptr;
}

ShaderBlock* GraphicsStorage::GetShaderStorageBuffer(const char * name)
{
	for (auto buffer : shaderStorageBuffers)
	{
		if (buffer->name.compare(name) == 0)
		{
			return buffer;
		}
	}
	return nullptr;
}

std::unordered_map<std::string, Vao*> GraphicsStorage::vaos;
std::unordered_map<std::string, Texture*> GraphicsStorage::textures;
std::unordered_map<std::string, TextureInfo*> GraphicsStorage::texturesToLoad;
std::unordered_map<std::string, TextureInfo*> GraphicsStorage::cubeMapsToLoad;
std::vector<Material*> GraphicsStorage::materials;
std::unordered_map<std::string, OBJ*> GraphicsStorage::objs;
std::unordered_map<std::string, Texture*> GraphicsStorage::cubemaps;
std::unordered_map<std::string, GLuint> GraphicsStorage::shaderIDs;
std::unordered_map<std::string, ShaderPaths> GraphicsStorage::shaderPaths;
std::unordered_map<std::string, Shader*> GraphicsStorage::shaders;
std::vector<ShaderBlock*> GraphicsStorage::uniformBuffers;
std::vector<ShaderBlockData*> GraphicsStorage::uniformBuffersDatas;
std::vector<ShaderBlock*> GraphicsStorage::shaderStorageBuffers;
std::vector<ShaderBlockData*> GraphicsStorage::shaderStoragesDatas;
std::map<std::string, std::string> GraphicsStorage::paths;