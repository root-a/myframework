#pragma once
#include <vector>
#include <unordered_map>
#include "CPUBlockData.h"

class ShaderBlock;
class DataRegistry;

class ShaderBlockData
{
public:
	ShaderBlockData(ShaderBlock* newUniformBuffer);
	~ShaderBlockData();
	void SetData(const char* uniformName, const void* newData, int size);
	void RegisterPropertyData(const char* uniformName, const void* newData, int size);
	void UpdateAndSubmit();
	void Submit();
	void RegisterProperties(const DataRegistry& dataRegistry);
	void RegisterDataWithOffsets(const DataRegistry& dataRegistry, std::unordered_map<std::string, int>& shaderBlockOffsets);
	void RegisterDataWithOffset(int offset, const void* data, int size);
	ShaderBlock* shaderBlock;
	std::vector<DataBinding> dataBindings;
private:
	int FindDataBindingIndex(int offset);
};