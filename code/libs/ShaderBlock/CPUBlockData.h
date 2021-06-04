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
	bool operator < (const DataBinding& str) const
	{
		return (offset < str.offset);
	}
};

class CPUBlockData
{
public:
	CPUBlockData();
	~CPUBlockData();
	void SetData(DataBinding& ub);
	void SetData(int offset, const void* data, int size);

	std::vector<char> data;
	void SetSize(int size);
	void UpdateBuffer(std::vector<DataBinding>& dataBindings);

	void ResetCounters();
	int gpuBufferStart = 0;
	void* cpuBufferStart = nullptr;
	int updateStart = INT_MAX;
	int updateEnd = INT_MIN;
	int sizeToUpdate = 0;

private:
	void UpdateCounters(int offset, int size);
};