#pragma once
#include <stdlib.h> 
#include "Chunk.h"

template <typename T, int chunk_length = 1000>
class PoolParty
{
typedef Chunk<T, chunk_length> C;
public:
	PoolParty() { init_number_of_chunks = 0; current_number_of_chunks = 0; first_chunk = nullptr; current_free_chunk = nullptr; nr_of_elements = 0; }

	~PoolParty()
	{
		C* curChunk = first_chunk;
		for (int i = 0; i < current_number_of_chunks; i++)
		{
			C* nextChunk = curChunk->next;
			for (int i = 0; i < curChunk->current_free_index; i+=sizeOfT)
			{
				((T*)&curChunk->data[i])->~T();
			}
			delete curChunk;
			curChunk = nextChunk;
		}
		destroyed = true;
	}

	C* GetFirstChunk() { return first_chunk; }
	C* GetLastChunk() { return current_free_chunk; }
	
	void CreatePoolParty(int num_of_chunks = 100)
	{
		C* previous_chunk = new C;
		first_chunk = previous_chunk;
		current_free_chunk = previous_chunk;
		init_number_of_chunks = num_of_chunks;
		current_number_of_chunks = init_number_of_chunks;
		for(int i = 1; i < init_number_of_chunks; i++)
		{
			C* next_chunk = new C;
			previous_chunk->next = next_chunk;
			previous_chunk = next_chunk;
		}
	}

	template<typename... ArgTypes>
	T* Alloc(ArgTypes... args)
	{
		T* element = nullptr;
		if (unusedObjects.size() > 0){
			element = unusedObjects.back();
			unusedObjects.pop_back();
			element->~T(); //because of dynamic allocations in the assets we have to call their destructors
			new(element) T{ std::forward<ArgTypes>(args)... };
			//*element = std::move(T{ std::forward<ArgTypes>(args)... });
		}
		else if(current_free_chunk->current_free_index < current_free_chunk->chunk_size){
			element = (T*)&current_free_chunk->data[current_free_chunk->current_free_index];
			current_free_chunk->current_free_index += sizeOfT;
			new(element) T{ std::forward<ArgTypes>(args)... };
		}
		else if(current_free_chunk->next != nullptr){
			current_free_chunk = current_free_chunk->next;
			element = (T*)&current_free_chunk->data[current_free_chunk->current_free_index];
			current_free_chunk->current_free_index += sizeOfT;
			new(element) T{ std::forward<ArgTypes>(args)... };
		}
		else{
			C* next_chunk = new C;
			current_free_chunk->next = next_chunk;
			current_free_chunk = next_chunk;
			current_number_of_chunks++;
			element = (T*)&current_free_chunk->data[current_free_chunk->current_free_index];
			current_free_chunk->current_free_index += sizeOfT;
			new(element) T{ std::forward<ArgTypes>(args)... };
		}
		nr_of_elements++;
		return element;
	}

	void Dealloc(T* element)
	{
		unusedObjects.push_back(element);
		nr_of_elements--;
	}

	struct iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = C*;
		using reference = T&;

		iterator(pointer ptr, Chunk<T, chunk_length>::Iterator chunkIterator, pointer lastChunk) : m_ptr(ptr), m_chunkIterator(chunkIterator), m_lastChunk(lastChunk) {}

		reference operator*() const { return *m_chunkIterator; }
		pointer operator->() { return m_chunkIterator; }
		iterator& operator++() {
			m_chunkIterator++;
			if (m_chunkIterator == m_ptr->end())
			{
				if (m_lastChunk != m_ptr)
				{
					m_ptr = m_ptr->next;
					m_chunkIterator = m_ptr->begin();
				}
			}
			return *this;
		}
		iterator operator++(int) {
			iterator tmp = *this;
			m_chunkIterator++;
			if (m_chunkIterator == m_ptr->end())
			{
				if (m_lastChunk != m_ptr)
				{
					m_ptr = m_ptr->next;
					m_chunkIterator = m_ptr->begin();
				}
			}
			return tmp; 
		}
		friend bool operator== (const iterator& a, const iterator& b) {
			return a.m_chunkIterator == b.m_chunkIterator;
		};
		friend bool operator!= (const iterator& a, const iterator& b) {
			return a.m_chunkIterator != b.m_chunkIterator;
		};

	private:
		pointer m_ptr;
		Chunk<T,chunk_length>::Iterator m_chunkIterator;
		pointer m_lastChunk;
	};

	iterator begin() { return iterator(first_chunk, first_chunk->begin(), current_free_chunk); }
	iterator end() { return iterator(current_free_chunk, current_free_chunk->end(), current_free_chunk); }

	unsigned int GetCapacity() { return current_number_of_chunks * chunk_length; }
	int GetCount() { return nr_of_elements; }
private:
	C* current_free_chunk;
	int init_number_of_chunks;
	int current_number_of_chunks;
	int nr_of_elements;
	C* first_chunk;
	std::vector<T*> unusedObjects;
	int sizeOfT = sizeof(T);
	bool destroyed = false;
};

//to work around fragmentation we can store iteration ranges whenever there are holes
//then when we move from one range to another and keep iterating