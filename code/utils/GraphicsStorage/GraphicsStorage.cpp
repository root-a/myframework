#include "GraphicsStorage.h"
#include "ShaderBlock.h"
#include "Material.h"
#include "Texture.h"
#include "RenderBuffer.h"
#include "OBJ.h"
#include "Vao.h"
#include <GL/glew.h>
#include "TextureProfile.h"
#include "MaterialProfile.h"
#include "ObjectProfile.h"
#include "DataRegistry.h"
#include "ShaderBlockData.h"

GraphicsStorage::GraphicsStorage()
{
}

GraphicsStorage::~GraphicsStorage()
{
}

void GraphicsStorage::ClearUniformBuffers()
{
	uniformBuffers.clear();
}

void GraphicsStorage::ClearUniformBuffersDatas()
{
	uniformBuffersDatas.clear();
}

void GraphicsStorage::ClearShaders()
{
	shaderIDs.clear();
	shaderPathsAndGuids.clear();
}

void GraphicsStorage::Clear()
{
	ClearShaders();
}

void GraphicsStorage::ClearRuntimeAssets()
{
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

std::unordered_map<std::string, TextureInfo*> GraphicsStorage::texturesToLoad;
std::unordered_map<std::string, TextureInfo*> GraphicsStorage::cubeMapsToLoad;
std::vector<RenderElement*> GraphicsStorage::renderingQueue;
std::unordered_map<std::string, GLuint> GraphicsStorage::shaderIDs;
std::unordered_map<std::string, std::string> GraphicsStorage::shaderPathsAndGuids;
std::vector<ShaderBlock*> GraphicsStorage::uniformBuffers;
std::vector<ShaderBlockData*> GraphicsStorage::uniformBuffersDatas;
std::vector<ShaderBlock*> GraphicsStorage::shaderStorageBuffers;
std::vector<ShaderBlockData*> GraphicsStorage::shaderStoragesDatas;
std::unordered_map<std::string, std::string> GraphicsStorage::shaderBlockTypes;
std::unordered_map<std::string, std::unordered_map<std::string, uniform_info_t>> GraphicsStorage::shaderBlockUniforms;
std::map<std::string, std::string> GraphicsStorage::paths;
std::unordered_map<DataRegistry*, std::unordered_map<std::string, std::unordered_set<std::string>>> GraphicsStorage::luaProperties;
std::unordered_map<DataRegistry*, std::unordered_map<std::string, std::unordered_map<std::string,std::string>>> GraphicsStorage::objectProperties;
std::unordered_map<std::string, std::vector<BufferLayout>> GraphicsStorage::bufferDefinitions;
AssetRegistry GraphicsStorage::assetRegistry;