#pragma once
#include <vector>
#include <functional>
#include "RenderElement.h"
#include "CPUBlockData.h"
#include "ShaderDataType.h"

class VertexArray;

class LocationLayout
{
public:
	std::string name;
	ShaderDataType::Type type;
	unsigned int size;
	unsigned int offset;
	bool normalized;
	unsigned int instancesPerAttribute = 0;

	LocationLayout() {};

	LocationLayout(ShaderDataType::Type type, const std::string& name, unsigned int instancesPerAttribute = 0, bool normalized = false)
		: name(name), type(type), size(ShaderDataType::Size(type)), offset(0), normalized(normalized), instancesPerAttribute(instancesPerAttribute)
	{
	}
};

class BufferLayout
{
public:
	BufferLayout() {}

	BufferLayout(std::initializer_list<LocationLayout> locationsIn)
		: locations(locationsIn)
	{
		CalculateOffsetsAndStride();
	}
	bool operator== (const BufferLayout& rightLayout) const {
		if (stride != rightLayout.stride) return false;
		if (offset != rightLayout.offset) return false;
		if (locations.size() != rightLayout.locations.size()) return false;
		for (auto& loc : locations)
		{
			for (auto& rightloc : rightLayout.locations)
			{
				if (loc.type != rightloc.type)
				{
					return false;
				}
			}
		}
		return true;
	}

	void AddLocationLayout(const LocationLayout& locationLayout)
	{
		locations.emplace_back(locationLayout);
		CalculateOffsetsAndStride();
	}
	unsigned int GetStride() const { return stride; }
	unsigned int GetOffset() const { return offset; }
	std::vector<LocationLayout>& GetLocations() { return locations; }
	std::vector<LocationLayout> locations;
	bool isDynamic;
	void CalculateOffsetsAndStride()
	{
		unsigned int Offset = 0;
		stride = 0;
		isDynamic = false;
		for (auto& location : locations)
		{
			location.offset = Offset;
			Offset += location.size;
			stride += location.size;
			if (location.instancesPerAttribute > 0)
				isDynamic = true;
		}
	}
private:
	
	unsigned int stride = 0;
	unsigned int offset = 0; //offset to first element, hmm we should probably expose that, something to thing about in the future
	void* data;
};

class VertexBuffer
{
public:
	unsigned int handle;
	unsigned int maxElementCount;
	BufferLayout layout;
	unsigned int activeCount;
	VertexBuffer();
	VertexBuffer(const void* data, unsigned int elementCount, const BufferLayout& layout);
	void Update(const void* newData, unsigned int offset = 0, int nrOfElementsToUpdate = -1);
	void Resize(const void* newData, unsigned int newElementCount);
	virtual ~VertexBuffer() {};
	std::string name;
private:

};

class VertexBufferDynamic : public VertexBuffer
{
public:
	VertexBufferDynamic(const void* data, unsigned int elementCount, const BufferLayout& layout);
	void SetData(int elementNr, LocationLayout& location, void* data);
	void SetData(int elementNr, void* data, int size);
	void SetElementData(int elementNr, void* data);
	void IncreaseInstanceCount();
	void SetData(std::vector<DataBinding>& dataBindings);
	void SetLocationData(void* data, int size, int offset);
	void SetLocationData(void* data, LocationLayout& location);
	void Update();
	void Resize(unsigned int newElementCount);
	~VertexBufferDynamic();
	char* cpuData;
	std::vector<VertexArray*> vaos;
	bool dirty = false;
private:
};
