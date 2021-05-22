#pragma once
#include <unordered_map>
#include "RenderElement.h"

class ShaderBlock;
enum class BlockType;


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
	void LoadUniforms();
	void LoadBlocks(BlockType type);
	unsigned int GetLocation(const char* uniformName);
	void SetRenderTarget(int slot, GLenum renderTarget);
	std::vector<ShaderBlock*> globalUniformBuffers;
	std::vector<ShaderBlock*> materialUniformBuffers;
	std::vector<ShaderBlock*> objectUniformBuffers;
	std::vector<ShaderBlock*> globalShaderStorageBuffers;
	std::vector<ShaderBlock*> materialShaderStorageBuffers;
	std::vector<ShaderBlock*> objectShaderStorageBuffers;
	std::vector<ShaderOutput> outputs;
	std::vector<GLenum> renderTargets;
	bool HasUniformBuffer(int index);
	bool HasShaderStorageBuffer(int index);
	void ClearBuffers();
	void Bind();
	void Execute();
private:
	static unsigned int currentID;
	int FindRenderTargetIndex(GLenum renderTarget);
};