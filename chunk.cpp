#include "include/chunk.h"
#include "include/memory.h"

void initChunk(Chunk* chunk){
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = nullptr;
    initValueArray(&chunk->constants);

    // RLE encoded line count initialize
    chunk->lineCapacity = 0;
    chunk->lineCount = 0;
    chunk->lines = nullptr;
}

void freeChunk(Chunk* chunk){
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    // FREE_ARRAY(int, chunk->lines, chunk->capacity);
    FREE_ARRAY(LineRLE, chunk->lines, chunk->lineCapacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line){
    if(chunk->capacity < chunk->count + 1){
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
        // chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    // chunk->lines[chunk->count] = line;
    chunk->count++;

    if(chunk->lineCount == 0 || chunk->lines[chunk->lineCount - 1].lineNumber != line){
        int oldLineCapacity = chunk->lineCapacity;

        if(chunk->lineCapacity < chunk->lineCount + 1){
            chunk->lineCapacity = GROW_CAPACITY(oldLineCapacity);
            chunk->lines = GROW_ARRAY(LineRLE, chunk->lines, oldLineCapacity, chunk->lineCapacity);
        }
        chunk->lines[chunk->lineCount].lineNumber = line;
        chunk->lines[chunk->lineCount].count = 1;
        chunk->lineCount++;
    } else {  
        chunk->lines[chunk->lineCount - 1].count++;
    }
}

int addConstant(Chunk* chunk, Value value){
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}

void writeConstant(Chunk* chunk, Value value, int line){
    int index = addConstant(chunk, value);

    if(index < 256) {
        // add normal size constant
        writeChunk(chunk, OP_CONSTANT, line);
        writeChunk(chunk, static_cast<uint8_t>(index), line);
    } else {
        // add 24 bit constant
        writeChunk(chunk, OP_CONSTANT_LONG, line); // operator
        writeChunk(chunk, (index >> 16) & 0xff, line); // high byte
        writeChunk(chunk, (index >> 8) & 0xff, line); // mid byte
        writeChunk(chunk, index &0xff, line); // low byte
    }
}

int getLine(Chunk* chunk, int instructionOffset){
    int instructionSeen = 0;
    for(int i = 0; i < chunk->lineCount; i++){
        instructionSeen += chunk->lines[i].count;
        if(instructionSeen > instructionOffset){
            return chunk->lines[i].lineNumber;
        }
    }
    return -1;
}