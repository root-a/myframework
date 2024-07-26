#include "DataRegistry.h"

DataRegistry::DataRegistry()
{
}

DataRegistry::~DataRegistry()
{
}

void DataRegistry::RegisterProperty(const std::string & name, const DataInfo * property)
{
	if (bindings.find(name) == bindings.end())
	{
		bindings[name] = *property;
	}
}

const DataInfo* DataRegistry::AddAndRegisterProperty(const char* name, const void * address, int size, PropertyType type)
{
	auto binding = bindings.find(name);
	if (binding != bindings.end())
	{
		if (binding->second.size != size || binding->second.type != type)
		{
			pb.RemoveProperty(name);
		}
	}
	DataInfo* info = pb.AddProperty(name, address, size, type);
	RegisterProperties(&pb);
	return info;
}

void DataRegistry::RegisterProperty(const std::string& name, void * address, int size, PropertyType type)
{
	if (bindings.find(name) == bindings.end())
	{
		bindings.try_emplace(name, address, size, type);
	}
}

void DataRegistry::UnregisterProperty(const char* name)
{
	auto result = bindings.find(name);
	if (result != bindings.end())
	{
		bindings.erase(result);
	}
	pb.RemoveProperty(name);
}

const void* DataRegistry::GetPropertyPtr(const char * name)
{
	auto binding = bindings.find(name);
	if (binding != bindings.end())
		return binding->second.dataAddress;
	return nullptr;
}

const DataInfo* DataRegistry::GetProperty(const char * name) const
{
	auto binding = bindings.find(name);
	if (binding != bindings.end())
		return &binding->second;
	return nullptr;
}

void DataRegistry::SetProperty(const char* name, void* newData)
{
	pb.SetData(name, newData);
}

void DataRegistry::RegisterProperties(PropertyBuffer* properties)
{
	for (auto& binding : properties->bindings)
	{
		bindings[binding.first] = binding.second.info;
	}
}

void DataRegistry::Clear()
{
	pb.Clear();
	bindings.clear();
}