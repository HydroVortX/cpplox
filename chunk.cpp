#include "include/chunk.h"
#include "include/memory.h"

Chunk::~Chunk(){
    freeArray<std::uint8_t>(code_, capacity_);
    freeArray<LineRLE>(lines_, lineCapacity_);
    freeValueArray(&constants_);
}

void Chunk::write(std::uint8_t byte, int line){
    if(capacity_ < count_ + 1){
        int oldCapacity = capacity_;
        capacity_ = growCapacity(oldCapacity);
        code_ = growArray(code_, oldCapacity, capacity_);
    }

    code_[count_] = byte;
    count_++;

    if(lineCount_ == 0 || lines_[lineCount_ - 1].lineNumber != line){
        int oldLineCapacity = lineCapacity_;

        if(lineCapacity_ < lineCount_ + 1){
            lineCapacity_ = growCapacity(oldLineCapacity);
            lines_ = growArray(lines_, oldLineCapacity, lineCapacity_);
        }
        lines_[lineCount_].lineNumber = line;
        lines_[lineCount_].count = 1;
        lineCount_++;
    } else {  
        lines_[lineCount_ - 1].count++;
    }
}

int Chunk::addConstant(Value value){
    writeValueArray(&constants_, value);
    return constants_.count - 1;
}

void Chunk::writeWithIndex(OpCode shortOp, OpCode longOp, int index, int line){
    if(index < 256) {
        // add normal size constant
        write(shortOp, line);
        write(static_cast<std::uint8_t>(index), line);
    } else {
        // add 24 bit constant
        write(longOp, line); // operator
        write(static_cast<std::uint8_t>((index >> 16) & 0xff), line); // high byte
        write(static_cast<std::uint8_t>((index >> 8) & 0xff), line); // mid byte
        write(static_cast<std::uint8_t>(index &0xff), line); // low byte
    }
}

void Chunk::writeConstant(Value value, int line){
    writeWithIndex(OP_CONSTANT, OP_CONSTANT_LONG, addConstant(value), line);
}

int Chunk::getLine(int instructionOffset) const{
    int instructionSeen = 0;
    for(int i = 0; i < lineCount_; i++){
        instructionSeen += lines_[i].count;
        if(instructionSeen > instructionOffset){
            return lines_[i].lineNumber;
        }
    }
    return -1;
}