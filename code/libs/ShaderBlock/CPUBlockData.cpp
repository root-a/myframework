#include "CPUBlockData.h"
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

void CPUBlockData::UpdateBuffer(std::vector<DataBinding>& dataBindings)
{
	for (auto& ub : dataBindings)
	{
		SetData(ub);
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
