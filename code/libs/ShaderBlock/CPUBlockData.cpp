#include "CPUBlockData.h"
#include "DataRegistry.h"
#include "GL/glew.h"

CPUBlockData::CPUBlockData()
{
}

CPUBlockData::~CPUBlockData()
{
}

void CPUBlockData::SetSize(int size)
{
	data.resize(size);
}

void CPUBlockData::RegisterDataWithOffsets(const DataRegistry& dataRegistry, std::unordered_map<std::string, int>& shaderBlockOffsets)
{
	for (auto& nameAndOffset : shaderBlockOffsets)
	{
		auto property = dataRegistry.GetData(nameAndOffset.first);
		if (property != nullptr)
		{
			//UpdateCounters(nameAndOffset.second, property->size); //should not have it here
			int index = FindDataBindingIndex(nameAndOffset.second);
			if (index == -1)
				dataBindings.push_back(DataBinding(nameAndOffset.second, property->dataAddress, property->size));
			else
			{
				dataBindings[index].offset = nameAndOffset.second;
				dataBindings[index].dataAddress = property->dataAddress;
				dataBindings[index].size = property->size;
			}
		}
	}
}

void CPUBlockData::RegisterDataWithOffset(int offset, const void* data, int size)
{
	int index = FindDataBindingIndex(offset);
	if (index == -1)
		dataBindings.push_back(DataBinding(offset, data, size));
	else
	{
		dataBindings[index].offset = offset;
		dataBindings[index].dataAddress = data;
		dataBindings[index].size = size;
	}
}

void CPUBlockData::UpdateBuffer()
{
	for (auto& ub : dataBindings)
	{
		SetData(ub);// memcpy(&data[ub.location], ub.dataAddress, ub.size);
	}
}

void CPUBlockData::SetData(DataBinding& ub)
{
	UpdateCounters(ub.offset, ub.size);
	memcpy(&data[ub.offset], ub.dataAddress, ub.size);
}

void CPUBlockData::SetData(int offset, const void* newData, int size)
{
	UpdateCounters(offset, size);
	memcpy(&data[offset], newData, size);
}

void CPUBlockData::ResetCounters()
{
	gpuBufferStart = 0;
	cpuBufferStart = nullptr;
	updateStart = INT_MAX;
	updateEnd = INT_MIN;
	sizeToUpdate = 0;
}

int CPUBlockData::FindDataBindingIndex(int offset)
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

void CPUBlockData::UpdateCounters(int newLocation, int size)
{
	if (newLocation < updateStart)
	{
		updateStart = newLocation;
		gpuBufferStart = newLocation;
		cpuBufferStart = &data[newLocation];
	}
	if (newLocation >= updateEnd)
	{
		updateEnd = newLocation + size;
	}
	sizeToUpdate = updateEnd - updateStart;
}
