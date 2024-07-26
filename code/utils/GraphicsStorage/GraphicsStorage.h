#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <string>
#include "Shader.h"
#include "PoolParty.h"
#include "Node.h"
#include "Bounds.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "InstanceSystem.h"
#include "FastInstanceSystem.h"
#include "RigidBody.h"
#include "AssetRegistry.h"
#include "Vbo.h"

class VertexArray;
class VertexBuffer;
class Material;
class OBJ;
class Texture;
class CPUBlockData;
class ShaderBlock;
class ShaderBlockData;
class RenderPass;
class RenderProfile;
class TextureProfile;
class MaterialProfile;
class ObjectProfile;
class DataRegistry;
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
	const char** newNames;
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
	static std::unordered_map<std::string, TextureInfo*> texturesToLoad;
	static std::unordered_map<std::string, TextureInfo*> cubeMapsToLoad;
	static std::unordered_map<std::string, std::vector<BufferLayout>> bufferDefinitions;
	static std::vector<ShaderBlock*> uniformBuffers;
	static std::vector<ShaderBlockData*> uniformBuffersDatas;
	static std::vector<ShaderBlock*> shaderStorageBuffers;
	static std::vector<ShaderBlockData*> shaderStoragesDatas;
	static std::unordered_map<std::string, std::string> shaderBlockTypes;
	static std::unordered_map<std::string, std::unordered_map<std::string, uniform_info_t>> shaderBlockUniforms;
	static std::unordered_map<std::string, GLuint> shaderIDs;
	static std::unordered_map<std::string, std::string> shaderPathsAndGuids;
	static std::vector<RenderElement*> renderingQueue;
	static std::map<std::string, std::string> paths;
	static std::unordered_map<DataRegistry*, std::unordered_map<std::string, std::unordered_set<std::string>>> luaProperties;
	static std::unordered_map<DataRegistry*, std::unordered_map<std::string, std::unordered_map<std::string,std::string>>> objectProperties;
	static void ClearUniformBuffers();
	static void ClearUniformBuffersDatas();
	static void ClearShaders();
	static void Clear();
	static void ClearRuntimeAssets();
	static ShaderBlock* GetUniformBuffer(int index);
	static ShaderBlockData* GetUniformBufferData(int index);
	static ShaderBlockData* GetShaderStorageData(int index);
	static ShaderBlock* GetShaderStorageBuffer(int index);

	static ShaderBlock* GetUniformBuffer(const char* name);
	static ShaderBlockData* GetUniformBufferData(const char* name);
	static ShaderBlockData* GetShaderStorageBufferData(const char* name);
	static ShaderBlock* GetShaderStorageBuffer(const char* name);
	static AssetRegistry assetRegistry;
private:
};



