#include <climits>
#include "Ebo.h"
#include <GL/glew.h>

ElementBuffer::ElementBuffer(const void* indices, unsigned int count)
{
	indicesType = GL_UNSIGNED_BYTE;
	unsigned char* indicesUB;
	unsigned short* indicesUS;
	unsigned int* indicesUI;
	indicesUB = (unsigned char*)indices;

	if (count > UCHAR_MAX)
	{
		indicesType = GL_UNSIGNED_SHORT;
		indicesUS = (unsigned short*)indices;
	}
	if (count > USHRT_MAX)
	{
		indicesType = GL_UNSIGNED_INT;
		indicesUI = (unsigned int*)indices;
	}

	indicesCount = count;

	glCreateBuffers(1, &handle);
	glNamedBufferStorage(handle, count * IndicesTypeSize(indicesType), indices, GL_DYNAMIC_STORAGE_BIT);
}

ElementBuffer::~ElementBuffer()
{
}

unsigned int ElementBuffer::IndicesTypeSize(unsigned int type)
{
	switch (type)
	{
	case GL_UNSIGNED_BYTE:	return sizeof(unsigned char);
	case GL_UNSIGNED_SHORT:	return sizeof(unsigned short);
	case GL_UNSIGNED_INT:	return sizeof(unsigned int);
	}
	return 0;
}

void ElementBuffer::Resize(const void* newData, unsigned int newCount)
{
	glDeleteBuffers(1, &handle);
	indicesType = GL_UNSIGNED_BYTE;
	unsigned char* indicesUB;
	unsigned short* indicesUS;
	unsigned int* indicesUI;
	indicesUB = (unsigned char*)newData;

	if (newCount > UCHAR_MAX)
	{
		indicesType = GL_UNSIGNED_SHORT;
		indicesUS = (unsigned short*)newData;
	}
	if (newCount > USHRT_MAX)
	{
		indicesType = GL_UNSIGNED_INT;
		indicesUI = (unsigned int*)newData;
	}

	indicesCount = newCount;

	glCreateBuffers(1, &handle);
	glNamedBufferStorage(handle, indicesCount * IndicesTypeSize(indicesType), newData, GL_DYNAMIC_STORAGE_BIT);
}