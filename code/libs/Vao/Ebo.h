#pragma once

class ElementBuffer
{
public:
	ElementBuffer(const void* indices, unsigned int count);
	~ElementBuffer();
	static unsigned int IndicesTypeSize(unsigned int type);
	void Resize(const void* newData, unsigned int newCount);
	unsigned int handle;
	unsigned int indicesType;
	unsigned int indicesCount;
private:

};