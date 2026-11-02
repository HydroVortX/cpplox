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

class Chunk {
    public:
        Chunk() = default;
        ~Chunk();
        Chunk(const Chunk&) = delete;
        Chunk& operator=(const Chunk&) = delete;

        void write(std::uint8_t byte, int line);
        void writeConstant(Value value, int line);
        int addConstant(Value value);
        int getLine(int instructionOffset) const;
        int count() const {return count_;}
        const std::uint8_t* code() const {return code_;}
        int constantCount() const {return constants_.count;}
        Value constantAt(int index) const {return constants_.values[index];}
    
    private:
        int count_ = 0;
        int capacity_ = 0;
        std::uint8_t* code_ = nullptr;
        ValueArray constants_;

        int lineCount_ = 0;
        int lineCapacity_ = 0;
        LineRLE* lines_ = nullptr;
};
