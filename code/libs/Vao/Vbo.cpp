#include "Vbo.h"
#include "Vao.h"
#include <GL/glew.h>

VertexBuffer::VertexBuffer(const void* newData, unsigned int newElementCount, const BufferLayout& bufferLayout)
{
	layout = bufferLayout;
	maxElementCount = newElementCount;
	activeCount = 0;
	glCreateBuffers(1, &handle);
	glNamedBufferStorage(handle, layout.GetStride() * maxElementCount, newData, 0);
}

void VertexBuffer::Update(const void* newData, unsigned int offset, int nrOfElementsToUpdate)
{
	if (nrOfElementsToUpdate > -1)
	{
		activeCount = nrOfElementsToUpdate;
	}
	glNamedBufferSubData(handle, layout.GetStride() * offset, layout.GetStride() * activeCount, newData);
}

void VertexBuffer::Resize(const void* newData, unsigned int newElementCount)
{
	maxElementCount = newElementCount;
	glDeleteBuffers(1, &handle);
	glCreateBuffers(1, &handle);
	glNamedBufferStorage(handle, layout.GetStride() * maxElementCount, newData, GL_DYNAMIC_STORAGE_BIT);
}

VertexBuffer::VertexBuffer()
{
	handle = -1;
	activeCount = 0;
	maxElementCount = 0;
}

VertexBufferDynamic::VertexBufferDynamic(const void* data, unsigned int elementCount, const BufferLayout& bufferLayout)
{
	layout = bufferLayout;
	maxElementCount = elementCount;
	activeCount = 0;
	glCreateBuffers(1, &handle);
	glNamedBufferStorage(handle, layout.GetStride() * maxElementCount, data, GL_DYNAMIC_STORAGE_BIT);
	cpuData = new char[layout.GetStride() * maxElementCount];
}

VertexBufferDynamic::~VertexBufferDynamic()
{
	delete[] cpuData;
};

void VertexBufferDynamic::SetLocationData(void* data, int size, int offset)
{
	memcpy(&cpuData[layout.GetStride() * activeCount + offset], data, size);
}

void VertexBufferDynamic::SetLocationData(void* data, LocationLayout& location)
{
	memcpy(&cpuData[layout.GetStride() * activeCount + location.offset], data, location.size);
}

void VertexBufferDynamic::SetElementData(int elementNr, void* data)
{
	memcpy(&cpuData[layout.GetStride() * elementNr], data, layout.GetStride());
}

void VertexBufferDynamic::SetData(int elementNr, LocationLayout& location, void* data) //could probably send Property containing offset and size, offset is calculated by buffer layout
{
	memcpy(&cpuData[layout.GetStride() * elementNr + location.offset], data, location.size);
}

void VertexBufferDynamic::SetData(int elementNr, void* data, int size)
{
	memcpy(&cpuData[layout.GetStride() * elementNr], data, size);
}

void VertexBufferDynamic::SetData(std::vector<DataBinding>& dataBindings)
{
	for (auto db : dataBindings)
	{
		memcpy(&cpuData[layout.GetStride() * activeCount + db.offset], db.dataAddress, db.size);
	}
}

void VertexBufferDynamic::IncreaseInstanceCount()
{
	activeCount++;
	Resize(activeCount);
}

void VertexBufferDynamic::Resize(unsigned int newElementCount)
{
	if (newElementCount > maxElementCount)
	{
		char* newData = new char[layout.GetStride() * newElementCount];
		memcpy(newData, cpuData, layout.GetStride() * maxElementCount);
		maxElementCount = newElementCount;
		delete[] cpuData;
		cpuData = newData;
		dirty = true;
	}
}

void VertexBufferDynamic::Update()
{
	if (dirty)
	{
		for (auto vao : vaos)
		{
			vao->ResizeVertexBuffer(this, cpuData, maxElementCount);
		}
		dirty = false;
	}
	else
	{
		VertexBuffer::Update(cpuData, layout.GetOffset());
	}
}