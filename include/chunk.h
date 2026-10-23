#pragma once

#include "common.h"
#include "value.h"

enum OpCode {
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_NIL,
    OP_NOT,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_RETURN,
};

struct LineRLE {
    int lineNumber = 0;
    int count = 0;
};

struct Chunk {
    int count = 0;
    int capacity = 0;
    std::uint8_t* code = nullptr;
    ValueArray constants;

    int lineCount = 0;
    int lineCapacity = 0;
    LineRLE* lines = nullptr;
};

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, std::uint8_t byte, int line);
void writeConstant(Chunk* chunk, Value value, int line);
int addConstant (Chunk* chunk, Value value);
int getLine(Chunk* chunk, int instructionOffset);
