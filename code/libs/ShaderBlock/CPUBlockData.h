#pragma once
#include <vector>
#include <unordered_map>

struct DataBinding
{
	DataBinding() { offset = -1; dataAddress = nullptr; size = 0; }
	DataBinding(int newOffset, const void* newDataAddress, int newSize) { offset = newOffset; dataAddress = newDataAddress; size = newSize; }
	int offset;
	const void* dataAddress;
	int size;
};

class DataRegistry;

class CPUBlockData
{
	typedef unsigned int GLuint;
	typedef unsigned int GLenum;
public:
	CPUBlockData();
	~CPUBlockData();
	void SetData(DataBinding& ub);
	void SetData(int offset, const void* data, int size);

	std::vector<char> data;
	void SetSize(int size);
	void RegisterDataWithOffsets(const DataRegistry& dataRegistry, std::unordered_map<std::string, int>& shaderBlockOffsets);
	void RegisterDataWithOffset(int offset, const void* data, int size);
	void UpdateBuffer();

	int gpuBufferStart = 0;
	void* cpuBufferStart = nullptr;
	void ResetCounters();
	int updateStart = INT_MAX;
	int updateEnd = INT_MIN;
	int sizeToUpdate = 0;

private:
	int FindDataBindingIndex(int offset);
	std::vector<DataBinding> dataBindings;
	void UpdateCounters(int newLocation, int size);
};