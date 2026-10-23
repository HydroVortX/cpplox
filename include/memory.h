#pragma once

#include "common.h"
#include "object.h"

void* reallocate(void* pointer, std::size_t oldSize, std::size_t newSize);
void freeObjects();

constexpr int growCapacity(int capacity){
    return capacity < 8 ? 8 : capacity * 2;
}

template <typename T>
T* allocate(std::size_t count){
    return static_cast<T*>(reallocate(nullptr, 0, sizeof(T) * (count)));
}

template <typename T>
T* growArray(T* pointer, int oldCount, int newCount){
    return static_cast<T*>(reallocate(pointer, sizeof(T) * (oldCount), sizeof(T) * (newCount)));
}

template <typename T>
void freeArray(T* pointer, int oldCount){
    reallocate(pointer, sizeof(T) * (oldCount), 0);
}

template <typename T>
void freeOne(T* pointer){
    reallocate(pointer, sizeof(T), 0);
}