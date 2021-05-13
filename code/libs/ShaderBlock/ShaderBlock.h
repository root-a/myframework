#pragma once
#include <vector>
#include <unordered_map>

class CPUBlockData;

enum BlockType
{
	Uniform, Storage
};

class ShaderBlock
{
	typedef unsigned int GLuint;
	typedef unsigned int GLenum;
public:
	ShaderBlock(int newSize, int newIndex, BlockType type);
	~ShaderBlock();
	void Bind();
	void Unbind();
	void Submit(CPUBlockData& buffer);
	void AddVariableOffset(const std::string& uniformName, int loc);
	std::unordered_map<std::string, int> offsets;
	std::string name;
	int index;
	int size;
private:
	void Generate();
	ShaderBlock();
	GLuint handle;
	GLenum target;
};