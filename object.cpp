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

static ObjString* allocateString(char* chars, int length){
    ObjString* string = static_cast<ObjString*>(allocateObject(sizeof(ObjString), OBJ_STRING));
    string->length = length;
    string->chars = chars;
    return string;
}

ObjString* takeString(char* chars, int length){
    return allocateString(chars, length);
}

ObjString* copyString(const char* chars, int length){
    char* heapChars = ALLOCATE(char, length + 1);
    std::memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length);
}

void printObject(Value value){
    switch(objType(value)){
        case OBJ_STRING:
            std::cout << asCString(value);
            break;
    }
}