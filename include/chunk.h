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
    int lineNumber;
    int count;
};

struct Chunk
{
    int count;
    int capacity;
    std::uint8_t* code;
    ValueArray constants;

    int lineCount;
    int lineCapacity;
    LineRLE* lines;
};

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, std::uint8_t byte, int line);
void writeConstant(Chunk* chunk, Value value, int line);
int addConstant (Chunk* chunk, Value value);
int getLine(Chunk* chunk, int instructionOffset);
