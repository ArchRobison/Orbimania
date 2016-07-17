#pragma once
#ifndef SimpleArray_H
#define SimpleArray_H

#include <initializer_list>

template<typename T>
class SimpleArray {
    T* array;
    size_t length;
    void operator=(const SimpleArray&) = delete;
    SimpleArray( const SimpleArray& ) = delete;
public:
    SimpleArray() : array(nullptr), length(0) {}
    SimpleArray(std::initializer_list<T> l) : length(l.size()) {
        array = new T[l.size()];
        size_t k = 0;
        for( auto i: l )
            array[k++] = i;
    }
    ~SimpleArray() {delete[] array;}
    size_t size() const {return length;}
    T& operator[]( size_t k ) {
        Assert(k<length);
        return array[k];
    }
    T* begin() const {return array;}
    T* end() const {return array+length;}
};

#endif /* SimpleArray_H */
