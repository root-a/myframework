#pragma once
#include <vector>
#include <unordered_map>
#include "CPUBlockData.h"
#include <string>

enum class BlockType
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
	void Submit();
	void AddVariableOffset(const std::string& uniformName, int loc);
	void SetData(const char* uniformName, const void* newData, int size);
	std::unordered_map<std::string, int> offsets;
	CPUBlockData data;
	std::string name;
	int index;
	int size;
	GLuint handle;
private:
	void Generate();
	ShaderBlock();
	GLenum target;
};