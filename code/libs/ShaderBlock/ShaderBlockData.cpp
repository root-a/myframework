#include "ShaderBlockData.h"
#include "CPUBlockData.h"
#include "ShaderBlock.h"
#include "DataRegistry.h"
#include <algorithm>

ShaderBlockData::ShaderBlockData(ShaderBlock* newUniformBuffer)
{
	shaderBlock = newUniformBuffer;
}

ShaderBlockData::~ShaderBlockData()
{
}

void ShaderBlockData::SetData(const char * uniformName, const void * newData, int size)
{
	shaderBlock->SetData(uniformName, newData, size);
}

void ShaderBlockData::RegisterPropertyData(const char* uniformName, const void* newData, int size)
{
	if (shaderBlock->offsets.find(uniformName) != shaderBlock->offsets.end())
	{
		RegisterDataWithOffset(shaderBlock->offsets[uniformName], newData, size);
	}
}

void ShaderBlockData::UpdateAndSubmit()
{
	shaderBlock->data.UpdateBuffer(dataBindings);
	if (shaderBlock->data.sizeToUpdate > 0) shaderBlock->Submit();
}

void ShaderBlockData::Submit()
{
	if (shaderBlock->data.sizeToUpdate > 0) shaderBlock->Submit();
}

void ShaderBlockData::RegisterProperties(const DataRegistry & dataRegistry)
{
	RegisterDataWithOffsets(dataRegistry, shaderBlock->offsets);
}

void ShaderBlockData::RegisterDataWithOffsets(const DataRegistry& dataRegistry, std::unordered_map<std::string, int>& shaderBlockOffsets)
{
	for (auto& nameAndOffset : shaderBlockOffsets)
	{
		auto property = dataRegistry.GetProperty(nameAndOffset.first.c_str());
		if (property != nullptr)
		{
			//UpdateCounters(nameAndOffset.second, property->size); //should not have it here
			int index = FindDataBindingIndex(nameAndOffset.second);
			if (index == -1)
				dataBindings.emplace_back(nameAndOffset.second, property->dataAddress, property->size);
			else
			{
				dataBindings[index].offset = nameAndOffset.second;
				dataBindings[index].dataAddress = property->dataAddress;
				dataBindings[index].size = property->size;
			}
		}
	}
	std::sort(dataBindings.begin(), dataBindings.end());
}

void ShaderBlockData::RegisterDataWithOffset(int offset, const void* data, int size)
{
	int index = FindDataBindingIndex(offset);
	if (index == -1)
		dataBindings.emplace_back(offset, data, size);
	else
	{
		dataBindings[index].offset = offset;
		dataBindings[index].dataAddress = data;
		dataBindings[index].size = size;
	}
	std::sort(dataBindings.begin(), dataBindings.end());
}

int ShaderBlockData::FindDataBindingIndex(int offset)
{
	for (size_t i = 0; i < dataBindings.size(); i++)
	{
		if (dataBindings[i].offset == offset)
		{
			return i;
		}
	}
	return -1;
}