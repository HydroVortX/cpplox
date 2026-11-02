#pragma once

#include "chunk.h"
#include "table.h"
#include "value.h"

struct VM {
    Chunk* chunk = nullptr;
    const std::uint8_t* ip = nullptr;
    int stackCapacity = 0;
    int stackCount = 0;
    Value* stack = nullptr;
    Value* stackTop = nullptr;
    Table strings;
    Obj* objects = nullptr;
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
