#pragma once

#include "common.h"

using Value = double;

struct ValueArray{
    int capacity;
    int count;
    Value* values;
};

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);