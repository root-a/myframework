#include "PropertyBuffer.h"
#include "DataInfo.h"
#include "DataRegistry.h"

PropertyBuffer::PropertyBuffer()
{
}

PropertyBuffer::~PropertyBuffer()
{
}

DataInfo* PropertyBuffer::AddProperty(const char* name, const void* newData, int dataSize, PropertyType type)
{
	data.resize(size + dataSize);
	memcpy(&data[size], newData, dataSize);
	int offset = size;
	bindings.try_emplace(name, offset, &data[size], dataSize, type);
	size += dataSize;
	for (auto& binding : bindings)
	{
		binding.second.info.dataAddress = &data[binding.second.offset];
	}
	return &bindings[name].info;
}

void PropertyBuffer::RemoveProperty(const char * name)
{
	auto& result = bindings.find(name);
	if (result != bindings.end())
	{
		int propertyLocation = result->second.offset;
		int propertySize = result->second.info.size;
		data.erase(data.begin() + result->second.offset, (data.begin() + result->second.offset + result->second.info.size));
		size -= result->second.info.size;
		bindings.erase(name);
		for (auto& binding : bindings)
		{
			if (binding.second.offset > propertyLocation)
			{
				binding.second.offset -= propertySize;
			}
			binding.second.info.dataAddress = &data[binding.second.offset];
		}
	}
}

const void* PropertyBuffer::GetPropertyPtr(const char* name)
{
	if (bindings.find(name) != bindings.end())
		return bindings[name].info.dataAddress;
	else
		return nullptr;
}

DataInfo* PropertyBuffer::GetProperty(const char* name)
{
	auto binding = bindings.find(name);
	if (binding != bindings.end())
		return &binding->second.info;
	else
		return nullptr;
}

void PropertyBuffer::SetData(const char* name, const void* newData)
{
	auto binding = bindings.find(name);
	if (binding != bindings.end())
	{
		auto db = binding->second;
		memcpy(db.info.dataAddress, newData, db.info.size);
	}
}

void PropertyBuffer::Clear()
{
	data.clear();
	size = 0;
	bindings.clear();
}
