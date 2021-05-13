#pragma once
#include <vector>
#include <unordered_map>

class CPUBlockData;
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
	ShaderBlock* shaderBlock;
	CPUBlockData* cpuBlock;
private:
};