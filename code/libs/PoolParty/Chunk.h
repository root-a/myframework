#pragma once
#include <stdlib.h> 

template <typename T, int length>
class Chunk
{
public:
	Chunk(){}
	~Chunk(){}
	
	T data[length];
	Chunk<T,length>* next = nullptr;
	
	int GetCurrentLength() { return current_free_index; }
	int current_free_index = 0;
	int data_size = length;
private:
	//Chunk();
};
