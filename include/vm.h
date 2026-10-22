#pragma once

#include "chunk.h"

#define STACK_MAX 256

struct VM {
    Chunk* chunk;
    std::uint8_t* ip;
    int stackCapacity;
    int stackCount;
    Value* stack;
    Value* stackTop;
    Obj* objects;
};

enum interpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
};

extern VM vm;

void initVM();
void freeVM();
interpretResult interpret(const char* source);
void push(Value value);
Value pop();
