#pragma once
#include <stdlib.h> 

template <typename T, int length>
class Chunk
{
public:
	Chunk(){}
	~Chunk(){}
	
	char data[length*sizeof(T)];
	Chunk<T,length>* next = nullptr;
	
	int GetCurrentLength() { return current_free_index; }
	int current_free_index = 0;
	int element_count = length;
	int element_size = sizeof(T);
	int chunk_size = length * element_size;

    struct Iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;

        Iterator(pointer ptr) : m_ptr(ptr) {}

        reference operator*() const { return *m_ptr; }
        pointer operator->() { return m_ptr; }
        Iterator& operator++() { m_ptr++; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
        friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

    private:
        pointer m_ptr;
    };

    Iterator begin() { return Iterator((T*)(&data[0])); }
    Iterator end() { return Iterator((T*)(&data[current_free_index])); }
private:
	//Chunk();
};
