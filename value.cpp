#include <iostream>
#include <cstring>

#include "include/memory.h"
#include "include/value.h"
#include "include/object.h"

void initValueArray(ValueArray* array){
    array->values = nullptr;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray* array, Value value){
    if(array->capacity < array->count + 1){
        int oldCapacity = array->capacity;
        array->capacity =  GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array){
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void printValue(Value value){
    switch(value.type){
        case VAL_BOOL:
            std::cout << (asBool(value) ? "true" : "false");
            break;
        case VAL_NIL:
            std::cout << "nil";
            break;
        case VAL_NUMBER:
            std::cout << asNumber(value);
            break;
        case VAL_OBJ: printObject(value); break;
    }
}

bool valuesEqual(Value a, Value b){
    if(a.type != b.type) return false;
    switch(a.type){
        case VAL_BOOL:   return asBool(a) == asBool(b);
        case VAL_NIL:    return true;
        case VAL_NUMBER: return asNumber(a) == asNumber(b);
        case VAL_OBJ: {
            ObjString* aString = asString(a);
            ObjString* bstring = asString(b);
            return aString->length == bstring->length && 
                std::memcmp(aString->chars, bstring->chars,
                            aString->length) == 0;
        }
        default:         return false;
    }
}