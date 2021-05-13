#pragma once
#include <unordered_map>
#include "RenderElement.h"

class RenderProfile;
class ShaderBlock;
class CPUBlockData;
class ShaderBlockData;
class Material;
class Matrix4F;
class Vector2F;
class Vector3F;
class Vector4F;
enum BlockType;


struct ShaderPaths
{
	std::string vs;
	std::string fs;
	std::string gs;
};

struct ShaderOutput
{
	int index;
	std::string type;
	std::string name;
};

class Shader : public RenderElement
{
	typedef unsigned int GLuint;
	typedef unsigned int GLenum;
public:
	Shader(unsigned int handle, std::string& sname, ShaderPaths& sp);
	~Shader();
	unsigned int shaderID;
	ShaderPaths shaderPaths;
	std::unordered_map<std::string, unsigned int> uniformLocations;
	void LoadUniforms();
	void LoadBlocks(BlockType type);
	unsigned int GetLocation(const char* uniformName);
	void UpdateUniform(const char* uniformName, Matrix4F& newValue);
	void UpdateUniform(const char* uniformName, const Vector2F& newValue);
	void UpdateUniform(const char* uniformName, const Vector3F& newValue);
	void UpdateUniform(const char* uniformName, const Vector4F& newValue);
	void UpdateUniform(const char* uniformName, const unsigned int newValue);
	void UpdateUniform(const char* uniformName, const int newValue);
	void UpdateUniform(const char* uniformName, const float newValue);
	void UpdateUniform(const char* uniformName, const bool newValue);
	void SetRenderTarget(int slot, GLenum renderTarget);
	std::vector<ShaderBlock*> globalUniformBuffers;
	std::vector<ShaderBlock*> materialUniformBuffers;
	std::vector<ShaderBlock*> objectUniformBuffers;
	std::vector<ShaderBlock*> globalShaderStorageBuffers;
	std::vector<ShaderBlock*> materialShaderStorageBuffers;
	std::vector<ShaderBlock*> objectShaderStorageBuffers;
	std::vector<ShaderOutput> outputs;
	std::vector<GLenum> renderTargets;
	void UpdateMaterialUniformBuffers(Material* mat);
	bool HasUniformBuffer(int index);
	bool HasShaderStorageBuffer(int index);
	void ClearBuffers();
	void Bind();
	void Execute();
private:
	static unsigned int currentID;
	int FindRenderTargetIndex(GLenum renderTarget);
};