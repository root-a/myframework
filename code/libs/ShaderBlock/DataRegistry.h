#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "DataInfo.h"
#include "PropertyBuffer.h"

class PropertyBuffer;
enum class PropertyType;

class DataRegistry
{
public:
	DataRegistry();
	~DataRegistry();
	const DataInfo* GetData(const std::string& name) const;
	void RegisterProperty(const std::string& name, const DataInfo* property);
	void RegisterProperty(const std::string& name, void* address, int size, PropertyType type);
	void UnregisterProperty(const char* name);
	const void* GetPropertyPtr(const char* name);
	const DataInfo* GetProperty(const char* name);
	void SetProperty(const char* name, void* newData);
	void AddAndRegisterProperty(const char* name, const void* address, int size, PropertyType type);
	void RegisterProperties(PropertyBuffer* properties);
	void Clear();
	std::unordered_map<std::string, DataInfo> bindings;
	PropertyBuffer pb;
private:
	std::vector<DataInfo*> dynamicallyAllocatedInfos;
};