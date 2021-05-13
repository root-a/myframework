#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "DataInfo.h"
#include "PropertyBuffer.h"

struct ExposedData
{
	ExposedData(std::string newName, const void* newAddress, int newSize) { name = newName; address = newAddress; size = newSize; }
	std::string name;
	const void* address;
	int size;
};

class PropertyBuffer;
enum class PropertyType;

//this is the data registry of an object where we register all properties that can be accessible to the material easily
//this just stores the existing properties
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
	void AddAndRegisterProperty(const char* name, const void* address, int size, PropertyType type);
	void RegisterProperties(PropertyBuffer* properties);
	void Clear();
	std::unordered_map<std::string, const DataInfo*> bindings;
	PropertyBuffer pb;
private:
};