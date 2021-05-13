#pragma once
enum class PropertyType
{
	INT,
	UINT,
	FLOAT,
	CHAR,
	BOOL,
	MAT2,
	MAT3,
	MAT4,
	VEC2,
	VEC3,
	VEC4
};



struct DataInfo
{
	DataInfo() { dataAddress = nullptr; size = 0; type = PropertyType::INT; };
	DataInfo(void* newAddress, int newSize, PropertyType newType) { dataAddress = newAddress; size = newSize; type = newType; }
	void* dataAddress;
	int size;
	PropertyType type;
	static const char* PropertyTypesAsString[11];
	static const int PropertyTypeSizes[11];
};