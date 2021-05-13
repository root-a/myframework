#include "ShaderBlock.h"
#include "CPUBlockData.h"
#include "GL/glew.h"

ShaderBlock::ShaderBlock()
{
}

ShaderBlock::ShaderBlock(int newSize, int newIndex, BlockType type)
{
	index = newIndex;
	size = newSize;
	if (type == Uniform) target = GL_UNIFORM_BUFFER;
	else target = GL_SHADER_STORAGE_BUFFER;
	Generate();
}

ShaderBlock::~ShaderBlock()
{
}

void ShaderBlock::Bind()
{
	glBindBuffer(target, handle);
}

void ShaderBlock::Unbind()
{
	glBindBuffer(target, 0);
}

void ShaderBlock::Generate()
{
	glCreateBuffers(1, &handle);
	glNamedBufferStorage(handle, size, NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(target, index, handle);
}

void ShaderBlock::Submit(CPUBlockData& buffer)
{
	glNamedBufferSubData(handle, buffer.gpuBufferStart, buffer.sizeToUpdate, buffer.cpuBufferStart);
	buffer.ResetCounters();
}

void ShaderBlock::AddVariableOffset(const std::string & uniformName, int loc)
{
	offsets[uniformName] = loc;
}

//void ShaderBlock::AddUniformData(std::string& uniformName, int dataSize)
//{
//	offsets[uniformName] = size;
//	size += dataSize;
//}