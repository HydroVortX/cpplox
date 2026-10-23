#include <cstdlib>

#include "include/memory.h"
#include "include/vm.h"

void* reallocate(void* pointer, std::size_t oldSize, std::size_t newSize){
    if(newSize == 0){
        std::free(pointer);
        return nullptr;
    }

    void* result = std::realloc(pointer, newSize);
    if(result == nullptr) std::exit(1);
    return result;
}

static void freeObject(Obj* object){
    switch(object->type){
        case OBJ_STRING: {
            ObjString* string = static_cast<ObjString*>(object);
            freeArray(string->chars, string->length + 1);
            freeOne(string);
            break;
        }
    }
}

void freeObjects(){
    Obj* object = vm.objects;
    while(object != nullptr){
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }
}