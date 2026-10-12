#pragma once

#include "chunk.h"

#define STACK_MAX 256

struct VM {
    Chunk* chunk;
    uint8_t* ip;
    int stackCapacity;
    int stackCount;
    Value* stack;
    Value* stackTop;
};

enum interpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
};

void initVM();
void freeVM();
interpretResult interpret(const char* source);
void push(Value value);
Value pop();
