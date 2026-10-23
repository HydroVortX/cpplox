#include <cstring>
#include <iostream>

#include "include/common.h"
#include "include/memory.h"
#include "include/object.h"
#include "include/value.h"
#include "include/vm.h"

static Obj* allocateObject(std::size_t size, ObjType type){
    Obj* object = static_cast<Obj*>(reallocate(nullptr, 0, size));
    object->type = type;

    object->next = vm.objects;
    vm.objects = object;
    return object;
}

static ObjString* allocateString(char* chars, int length, std::uint32_t hash){
    ObjString* string = static_cast<ObjString*>(allocateObject(sizeof(ObjString), OBJ_STRING));
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    return string;
}

static std::uint32_t hashString(const char* key, int length){
    std::uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++){
        hash ^= static_cast<std::uint8_t>(key[i]);
        hash *= 16777619;
    }
    return hash;
}

ObjString* takeString(char* chars, int length){
    std::uint32_t hash = hashString(chars, length);
    return allocateString(chars, length, hash);
}

ObjString* copyString(const char* chars, int length){
    std::uint32_t hash = hashString(chars, length);
    char* heapChars = allocate<char>(length + 1);
    std::memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length, hash);
}

void printObject(Value value){
    switch(objType(value)){
        case OBJ_STRING:
            std::cout << asCString(value);
            break;
    }
}