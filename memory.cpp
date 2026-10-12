#include <cstdlib>

#include "include/memory.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize){
    if(newSize == 0){
        std::free(pointer);
        return nullptr;
    }

    void* result = std::realloc(pointer, newSize);
    if(result == nullptr) std::exit(1);
    return result;
}