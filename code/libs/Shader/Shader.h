#pragma once
#include "RenderElement.h"
#include "ShaderDataType.h"

class RenderProfile;
class ShaderBlock;
class CPUBlockData;
class ShaderBlockData;
class Material;
class Matrix4F;
class Vector2F;
class Vector3F;
class Vector4F;
enum class BlockType;


struct ShaderPaths
{
	std::string vs;
	std::string fs;
	std::string gs;
	std::string path;
};

struct ShaderOutput
{
	int index;
	ShaderDataType::Type type;
	std::string name;
};

struct ShaderSampler
{
	int index;
	std::string type;
	std::string name;
};

struct uniform_info_t
{
	union
	{
		struct { int type, count, offset, blockIndex, arrayStride, matrixStride, isRowMajor, atomicCounterBufferIndex, location, size; };
		int properties[10];
	};
	std::string name;
};

class Shader : public RenderElement
{
	typedef unsigned int GLuint;
	typedef unsigned int GLenum;
public:
	Shader(unsigned int handle, const std::string& sname, const ShaderPaths& sp);
	~Shader();
	unsigned int shaderID;
	ShaderPaths shaderPaths;
	void LoadUniforms();
	void SetRenderTarget(int slot, GLenum renderTarget);
	std::vector<ShaderBlock*> globalUniformBuffers;
	std::vector<ShaderBlock*> materialUniformBuffers;
	std::vector<ShaderBlock*> objectUniformBuffers;
	std::vector<ShaderBlock*> globalShaderStorageBuffers;
	std::vector<ShaderBlock*> materialShaderStorageBuffers;
	std::vector<ShaderBlock*> objectShaderStorageBuffers;
	std::vector<ShaderOutput> outputs;
	std::vector<ShaderOutput> attributes;
	std::vector<ShaderSampler> samplers;
	std::vector<GLenum> renderTargets;
	bool HasUniformBuffer(int index);
	bool HasShaderStorageBuffer(int index);
	void Clear();
	void Bind();
	void Execute();
private:
	static unsigned int currentID;
	int FindRenderTargetIndex(GLenum renderTarget);
};