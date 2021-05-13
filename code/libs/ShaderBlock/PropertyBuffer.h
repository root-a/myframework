#pragma once
#include <vector>
#include <unordered_map>

struct DataInfo;
class DataRegistry;
enum class PropertyType;

struct DataInfoLoc
{
	DataInfoLoc() { int offset = 0; info = nullptr;};
	DataInfoLoc(int newLocation, DataInfo* newInfo) { offset = newLocation;  info = newInfo;}
	int offset;
	DataInfo* info;
};
class DataRegistry;

//should be a buffer that contains only it's properties, no existing properties

class PropertyBuffer
{
public:
	PropertyBuffer();
	~PropertyBuffer();
	
	DataInfo* AddProperty(const char* name, const void* data, int size, PropertyType type);
	void RemoveProperty(const char* name);
	const void* GetPropertyPtr(const char* name);
	DataInfo* GetProperty(const char* name);
	void SetData(const char* name, const void* data);
	void SetData(DataInfo* binding, const void * newData);
	void Clear();
	std::vector<char> data;
	std::unordered_map<std::string, DataInfoLoc> bindings;
private:
	int size = 0;
};