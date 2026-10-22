#include <iostream>
#include <iomanip>
#include <format>

#include "include/debug.h"
#include "include/value.h"

static int simpleInstruction(const char* name, int offset){
    std::cout << std::format("{}\n", name);
    return offset + 1;
}

static int constantInstruction(const char* name, Chunk* chunk, int offset){
    uint8_t constant = chunk->code[offset + 1];
    std::cout << std::format("{:<16} {:4} '", name, constant);
    printValue(chunk->constants.values[constant]);
    std::cout << "'\n";
    return offset + 2;
}

static int constantLongInstruction(const char* name, Chunk* chunk, int offset){
    int constant = (chunk->code[offset + 1] << 16) | (chunk->code[offset + 2] << 8) | (chunk->code[offset + 3]);

    std::cout << std::format("{:<16} {:4} '", name, constant);
    printValue(chunk->constants.values[constant]);
    std::cout << "'\n";
    return offset + 4;
}

void disassembleChunk(Chunk* chunk, const char* name){
    std::cout << "== " << name << " ==\n";

    for(int offset = 0; offset < chunk->count;){
        offset = disassembleInstruction(chunk, offset);
    }
}

int disassembleInstruction(Chunk* chunk, int offset){
    std::cout << std::format("{:04} ", offset);

    if(offset > 0 && getLine(chunk, offset) == getLine(chunk, offset -1)){
        std::cout << "   | ";
    } else {
        std::cout << std::format("{:4} ", getLine(chunk, offset));
    }

    std::uint8_t instruction = chunk->code[offset];
    switch(instruction){
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_CONSTANT_LONG:
            return constantLongInstruction("OP_CONSTANT_LONG", chunk, offset);
        case OP_NIL:
            return simpleInstruction("OP_NIL", offset);
        case OP_TRUE:
            return simpleInstruction("OP_TRUE", offset);
        case OP_FALSE:
            return simpleInstruction("OP_FALSE", offset);
        case OP_EQUAL:
            return simpleInstruction("OP_EQUAL", offset);
        case OP_GREATER:
            return simpleInstruction("OP_GREATER", offset);
        case OP_LESS:
            return simpleInstruction("OP_LESS", offset);
        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);
        case OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);
        case OP_NOT:
            return simpleInstruction("OP_NOT", offset);
        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        default:
            std::cout << std::format("Unknown opcode {}\n", instruction);
            return offset +1;
    }
}