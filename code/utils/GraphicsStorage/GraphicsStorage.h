#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include "Shader.h"
class Vao;
class Material;
class OBJ;
class Texture;
class CPUBlockData;
class ShaderBlock;
class ShaderBlockData;
class ObjectProfile;
class FrameBuffer;
class Object;
class Component;
class Camera;
struct TextureInfo;
class RenderBuffer;

struct AssetProperty
{
	const char* name;
	const char* type;
	const void* value;
};

struct AssetProperties 
{
	AssetProperty* properties;
	int size;
};

struct LuaProperties
{
	const char** properties;
	int size;
};

struct ObjectProperties
{
	const char* object;
	const char** properties;
	int size;
};

struct ObjectsProperties
{
	ObjectProperties* properties;
	int size;
};

class GraphicsStorage
{
	typedef unsigned int GLuint;
public:
	GraphicsStorage();
	~GraphicsStorage();
	static std::vector<Material*> materials;
	static std::unordered_map<std::string, Vao*> vaos;
	static std::unordered_map<std::string, Texture*> textures;
	static std::unordered_map<std::string, Texture*> cubemaps;
	static std::unordered_map<std::string, TextureInfo*> texturesToLoad;
	static std::unordered_map<std::string, TextureInfo*> cubeMapsToLoad;
	static std::unordered_map<std::string, OBJ*> objs;
	static std::unordered_map<std::string, GLuint> shaderIDs;
	static std::unordered_map<std::string, ShaderPaths> shaderPaths;
	static std::unordered_map<std::string, Shader*> shaders;
	static std::vector<ShaderBlock*> uniformBuffers;
	static std::vector<ShaderBlockData*> uniformBuffersDatas;
	static std::vector<ShaderBlock*> shaderStorageBuffers;
	static std::vector<ShaderBlockData*> shaderStoragesDatas;
	static std::map<std::string, std::string> GraphicsStorage::paths;
	static void ClearMaterials();
	static void ClearVaos();
	static void ClearTextures();
	static void ClearUniformBuffers();
	static void ClearUniformBuffersDatas();
	static void ClearCubemaps();
	static void ClearOBJs();
	static void ClearShaders();
	static void Clear();
	static ShaderBlock* GetUniformBuffer(int index);
	static ShaderBlockData* GetUniformBufferData(int index);
	static ShaderBlockData* GetShaderStorageData(int index);
	static ShaderBlock* GetShaderStorageBuffer(int index);

	static ShaderBlock* GetUniformBuffer(const char* name);
	static ShaderBlockData* GetUniformBufferData(const char* name);
	static ShaderBlockData* GetShaderStorageBufferData(const char* name);
	static ShaderBlock* GetShaderStorageBuffer(const char* name);
private:

};