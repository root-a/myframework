#pragma once
#include <stdlib.h> 
#include "Chunk.h"

template <typename T, int chunk_length = 5>
class PoolParty
{
typedef Chunk<T, chunk_length> C;
public:
	PoolParty(){}

	~PoolParty()
	{
		C* curChunk = firstChunk;
		for (int i = 0; i < init_number_of_chunks; i++)
		{
			C* nextChunk = curChunk->next;
			delete curChunk;
			curChunk = nextChunk;
		}
	}

	C* GetFirstChunk() { return firstChunk; }
	C* GetLastChunk() { return current_free_chunk; }
	
	void CreatePoolParty(int num_of_chunks = 100)
	{
		C* previous_chunk = new C;
		firstChunk = previous_chunk;
		current_free_chunk = previous_chunk;
		init_number_of_chunks = num_of_chunks;
		for(int i = 1; i < init_number_of_chunks; i++)
		{
			C* next_chunk = new C;
			previous_chunk->next = next_chunk;
			previous_chunk = next_chunk;
		}
	}

	T* PoolPartyAlloc()
	{
		if(current_free_chunk->current_free_index < current_free_chunk->data_size){
			T* element = &current_free_chunk->data[current_free_chunk->current_free_index];
			current_free_chunk->current_free_index++;
			return element;
		}
		else if(current_free_chunk->next != nullptr){
			current_free_chunk = current_free_chunk->next;
			T* element = &current_free_chunk->data[current_free_chunk->current_free_index];
			current_free_chunk->current_free_index++;
			return element;
		}
		else{
			C* next_chunk = new C;
			current_free_chunk->next = next_chunk;
			current_free_chunk = next_chunk;
			init_number_of_chunks++;
			T* element = &current_free_chunk->data[current_free_chunk->current_free_index];
			current_free_chunk->current_free_index++;
			return element;
		}
	}
private:
	C* current_free_chunk;
	int init_number_of_chunks;
	C* firstChunk;
};

