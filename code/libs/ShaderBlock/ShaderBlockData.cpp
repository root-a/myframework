#include "ShaderBlockData.h"
#include "CPUBlockData.h"
#include "ShaderBlock.h"

ShaderBlockData::ShaderBlockData(ShaderBlock* newUniformBuffer)
{
	shaderBlock = newUniformBuffer;
	cpuBlock = new CPUBlockData();
	cpuBlock->SetSize(shaderBlock->size);
}

ShaderBlockData::~ShaderBlockData()
{
	delete cpuBlock;
}

void ShaderBlockData::SetData(const char * uniformName, const void * newData, int size)
{
	if (shaderBlock->offsets.find(uniformName) != shaderBlock->offsets.end())
	{
		cpuBlock->SetData(shaderBlock->offsets[uniformName], newData, size);
	}
}

void ShaderBlockData::RegisterPropertyData(const char* uniformName, const void* newData, int size)
{
	if (shaderBlock->offsets.find(uniformName) != shaderBlock->offsets.end())
	{
		cpuBlock->RegisterDataWithOffset(shaderBlock->offsets[uniformName], newData, size);
	}
}

void ShaderBlockData::UpdateAndSubmit()
{
	cpuBlock->UpdateBuffer();
	shaderBlock->Submit(*cpuBlock);
}

void ShaderBlockData::Submit()
{
	shaderBlock->Submit(*cpuBlock);
}

void ShaderBlockData::RegisterProperties(const DataRegistry & dataRegistry)
{
	cpuBlock->RegisterDataWithOffsets(dataRegistry, shaderBlock->offsets);
}