#include "DataRegistry.h"

DataRegistry::DataRegistry()
{
}

DataRegistry::~DataRegistry()
{
	for (auto binding : bindings)
	{
		delete binding.second;
	}
}

const DataInfo* DataRegistry::GetData(const std::string & name) const
{
	auto& binding = bindings.find(name);
	if (binding != bindings.end())
	{
		return binding->second;
	}
	/* //there is no need to search in pb because everything is registered in bindings even added properties that reside inside pb
	else
	{
		auto& binding = pb.bindings.find(name);
		if (binding != pb.bindings.end())
		{
			return binding->second.info;
		}
	}
	*/
	return nullptr;
}

void DataRegistry::RegisterProperty(const std::string & name, const DataInfo * property)
{
	if (bindings.find(name) == bindings.end())
	{
		bindings[name] = property;
	}
}

void DataRegistry::AddAndRegisterProperty(const char* name, const void * address, int size, PropertyType type)
{
	auto& binding = bindings.find(name);
	if (binding != bindings.end())
	{
		if (binding->second->size != size || binding->second->type != type)
		{
			pb.RemoveProperty(name);
		}
	}
	bindings[name] = pb.AddProperty(name, address, size, type);
}

void
DataRegistry::RegisterProperty(const std::string& name, void * address, int size, PropertyType type)
{
	if (bindings.find(name) == bindings.end())
	{
		bindings[name] = new DataInfo(address, size, type);
	}
}

void DataRegistry::UnregisterProperty(const char* name)
{
	auto& result = bindings.find(name);
	if (result != bindings.end())
	{
		bindings.erase(result);
	}
	pb.RemoveProperty(name);
}

const void*
DataRegistry::GetPropertyPtr(const char * name)
{
	auto& binding = bindings.find(name);
	if (binding != bindings.end())
		return binding->second->dataAddress;
	/* //there is no need to search in pb because everything is registered in bindings even added properties that reside inside pb
	else
	{
		auto& binding = pb.bindings.find(name);
		if (binding != pb.bindings.end())
		{
			return binding->second.info->dataAddress;
		}
	}
	*/
	return nullptr;
}

const DataInfo * DataRegistry::GetProperty(const char * name)
{
	auto binding = bindings.find(name);
	if (binding != bindings.end())
		return binding->second;
	/* //there is no need to search in pb because everything is registered in bindings even added properties that reside inside pb
	else
	{
		auto& binding = pb.bindings.find(name);
		if (binding != pb.bindings.end())
		{
			return binding->second.info;
		}
	}
	*/
	return nullptr;
}

void
DataRegistry::RegisterProperties(PropertyBuffer* properties)
{
	for (auto& binding : properties->bindings)
	{
		bindings[binding.first] = binding.second.info;
	}
}

void
DataRegistry::Clear()
{
	//we need to property delete properties from memory!
	pb.Clear();
	bindings.clear();
}