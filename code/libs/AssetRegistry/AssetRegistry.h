#pragma once
#include "PropertyBuffer.h"
#include "DataRegistry.h"
#include <random>
#include "XoshiroCpp.hpp"
#include "uuid.h"
#include "PoolParty.h"
#include <typeinfo>
#include <any>
#include <typeindex>
#include <map>

static std::random_device random_device_seed_generator;
static XoshiroCpp::Xoshiro256PlusPlus generator(random_device_seed_generator());
static uuids::basic_uuid_random_generator<XoshiroCpp::Xoshiro256PlusPlus> gen(&generator);

class IDGenerator
{
public:

	IDGenerator() {};
	~IDGenerator() {};
	uuids::uuid GetNewEntityID()
	{
		return gen();
	};

	void ReleaseEntityID(unsigned int id)
	{
		unusedEntityIDs.push_back(id);
	}
private:
	unsigned int currentAvailableEntitiesID;
	std::vector<unsigned int> unusedEntityIDs;
};

class AssetRegistry
{
public:
	AssetRegistry() {};
	~AssetRegistry() { };
	template <typename T, int chunkSize = 1000>
	PoolParty<T,chunkSize>* RegisterType(int chunkCount = 10)
	{
		auto res = registry.find(typeid(T));
		if (res == registry.end())
		{
			auto emplaceRes = registry.emplace(typeid(T), PoolParty<T, chunkSize>());
			auto* pp = &std::any_cast<PoolParty<T, chunkSize>&>(emplaceRes.first->second);
			pp->CreatePoolParty(chunkCount);
			//entitiesIds.reserve(pp->GetCapacity());
			//iDsEntities.reserve(pp->GetCapacity());
			return pp;
		}
		else
		{
			return &std::any_cast<PoolParty<T, chunkSize>&>(res->second);
		}
	}

	template <typename T, int chunkSize = 1000>
	void UnregisterType()
	{
		auto res = registry.find(typeid(T));
		if (res != registry.end())
		{
			//delete res->second;
			//auto pp = std::any_cast<PoolParty<T, chunkSize>*>(res->second);
			//delete pp;
			registry.erase(res);
		}
	}

	template <typename T, int chunkSize = 1000>
	void ClearType()
	{
		UnregisterType<T, chunkSize>();
		RegisterType<T, chunkSize>();
	}

	template <typename T, int chunkSize = 1000>
	bool PoolPartyCount()
	{
		return registry[typeid(T)].GetCount();
	}

	template <typename T, int chunkSize = 1000>
	PoolParty<T, chunkSize>* GetPool()
	{
		auto res = registry.find(typeid(T));
		if (res != registry.end())
		{
			return &std::any_cast<PoolParty<T, chunkSize>&>(res->second);
		}
		return nullptr;
	}

	/*
	template <typename T, int chunkSize = 1000>
	PoolParty<T, chunkSize>* GetFactory()
	{
		auto res = registry.find(typeid(T));
		if (res != registry.end())
		{
			return &std::any_cast<PoolParty<T, chunkSize>&>(res->second);
		}
		return nullptr;
	}
	*/

	void Clear()
	{
		registry.clear();
	}

	template<typename T, int chunkSize = 1000, typename... ArgTypes>
	T* AllocAsset(ArgTypes... args)
	{
		auto& type = typeid(T);
		auto result = registry.find(type);
		T* asset = nullptr;
		if (result == registry.end())
		{
			this->RegisterType<T>();
			asset = std::any_cast<PoolParty<T, chunkSize>&>(registry[type]).Alloc(std::forward<ArgTypes>(args)...);
		}
		else
		{
			asset = std::any_cast<PoolParty<T, chunkSize>&>(result->second).Alloc(std::forward<ArgTypes>(args)...);
		}
		uuids::uuid generatedID = gen();
		entitiesIds[asset] = generatedID;
		iDsEntities[generatedID] = asset;
		return asset;
	}

	template<typename T, int chunkSize = 1000>
	void DeallocAsset(T* asset)
	{
		auto res = entitiesIds.find(asset);
		if (res != entitiesIds.end())
		{
			uuids::uuid id = res->second;
			entitiesIds.erase(res);
			std::any_cast<PoolParty<T, chunkSize>&>(registry[typeid(T)]).Dealloc(asset);
			auto res2 = iDsEntities.find(id);
			iDsEntities.erase(res2);
		}
	}

	std::unordered_map<std::type_index, std::any> registry;

	//this should be part of asset factory, not the registry:
	//it's nice idea though, this way it does not matter what type the asset is
	//but when the asset is removed we would have to remove it from here too
	//also when the type is removed we would have to remove all assets of that type as well
	//in other systems uuids are used as "object"
	//we already have that with our objects
	//delete uuid/object and all it's related stuff is deleted
	//difference here is that we could have properties on components etc
	//good thing about this solution is that the scripts could easily access properties
	std::map<uuids::uuid, DataRegistry> attributes;
	const DataInfo* GetProperty(const uuids::uuid& id, const char* attrName)
	{
		auto res = attributes.find(id);
		if (res != attributes.end())
		{
			return res->second.GetProperty(attrName);
		}
		return nullptr;
	}

	const DataInfo* SetProperty(const uuids::uuid& id, const char* attrName, void* data, int size, PropertyType type)
	{
		return attributes[id].AddAndRegisterProperty(attrName, data, size, type);
	}



	template<typename T, int chunkSize = 1000, typename... ArgTypes>
	T* AllocAssetWithUUID(const uuids::uuid& id, ArgTypes... args)
	{
		auto asset = (T*)GetAssetByID(id);
		if (asset == nullptr)
		{
			auto& type = typeid(T);
			auto result = registry.find(type);
			T* asset = nullptr;
			if (result == registry.end())
			{
				this->RegisterType<T>();
				asset = std::any_cast<PoolParty<T, chunkSize>&>(registry[type]).Alloc(std::forward<ArgTypes>(args)...);
			}
			else
			{
				asset = std::any_cast<PoolParty<T, chunkSize>&>(result->second).Alloc(std::forward<ArgTypes>(args)...);
			}
			entitiesIds[asset] = id;
			iDsEntities[id] = asset;
			return asset;
		}
		else
		{
			return asset;
		}
	}

	template<typename T, int chunkSize = 1000, typename... ArgTypes>
	T* AllocAssetWithStrUUID(const std::string& strId, ArgTypes... args)
	{
		auto id = uuids::uuid::from_string(strId);
		if (id.has_value())
		{
			return this->AllocAssetWithUUID<T>(id.value(), std::forward<ArgTypes>(args)...);
		}
		else
		{
			return nullptr;
		}
	}

	void* GetAssetByID(const uuids::uuid& id)
	{
		auto res = iDsEntities.find(id);
		if (res != iDsEntities.end())
			return res->second;
		else
			return nullptr;
	}

	void* GetAssetByStringID(const std::string& stringId)
	{
		auto id = uuids::uuid::from_string(stringId);
		if (id.has_value())
		{
			auto res = iDsEntities.find(id.value());
			if (res != iDsEntities.end())
				return res->second;
			else
				return nullptr;
		}
		else
		{
			return nullptr;
		}
	}

	uuids::uuid GetAssetID(void* asset)
	{
		auto res = entitiesIds.find(asset);
		if (res != entitiesIds.end())
			return res->second;
		else
			return uuids::uuid();
	}

	std::string GetAssetIDAsString(void* asset)
	{
		auto res = entitiesIds.find(asset);
		if (res != entitiesIds.end())
			return uuids::to_string(res->second);
		else
			return std::string();
	}

	//still accessible via pool
	//typename PoolParty<T, chunkSize>::iterator begin() { return pool.begin(); }
	//typename PoolParty<T, chunkSize>::iterator end() { return pool.end(); }

	//using AssetType = std::map<uuids::uuid, void*>;
	//typename AssetType::iterator begin() { return iDsEntities.begin(); }
	//typename AssetType::iterator end() { return iDsEntities.end(); }

private:
	std::map<void*, uuids::uuid> entitiesIds;
	std::map<uuids::uuid, void*> iDsEntities;
};