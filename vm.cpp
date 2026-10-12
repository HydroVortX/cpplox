#include <iostream>

#include "include/common.h"
#include "include/vm.h"
#include "include/debug.h"
#include "include/memory.h"
#include "include/compiler.h"

VM vm;

static void resetStack(){
    vm.stackTop = vm.stack;
}

void initVM(){
    resetStack();
}

void freeVM(){

}

void push(Value value){
    if(vm.stackCapacity < vm.stackCount + 1){
        int oldcapacity = vm.stackCapacity;
        vm.stackCapacity = GROW_CAPACITY(oldcapacity);
        vm.stack = GROW_ARRAY(Value, vm.stack, oldcapacity, vm.stackCapacity);
        vm.stackTop = vm.stack + vm.stackCount;
    }
    *vm.stackTop = value;
    vm.stackCount++;
    vm.stackTop++;
}

Value pop(){
    vm.stackTop--;
    vm.stackCount--;
    return *vm.stackTop;
}

static interpretResult run(){
    #define READ_BYTE() (*vm.ip++)
    #define READ_24BIT() (vm.ip += 3, ((vm.ip[-3]) << 16| (vm.ip[-2]) << 8 | vm.ip[-1]))
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    #define READ_CONSTANT_LONG() (vm.chunk->constants.values[READ_24BIT()])
    #define BINARY_OP(op) \
        do { \
            double b = pop(); \
            double a = pop(); \
            push(a op b); \
        } while(false)

        for(;;){
            #ifdef DEBUG_TRACE_EXECUTION
                std::cout << "          ";
                for(Value* slot = vm.stack; slot < vm.stackTop; slot++){
                    std::cout << "["; 
                    printValue(*slot);
                    std::cout << "]";
                }
                std::cout << "\n";
                disassembleInstruction(vm.chunk, static_cast<int>(vm.ip - vm.chunk->code));
            #endif
            uint8_t instruction;
            switch (instruction = READ_BYTE()) {
                case OP_CONSTANT_LONG: {
                    Value constant = READ_CONSTANT_LONG();
                    push(constant);
                    printValue(constant);
                    std::cout << "\n";
                    break;
                }
                case OP_CONSTANT: {
                    Value constant = READ_CONSTANT();
                    push(constant);
                    printValue(constant);
                    std::cout << "\n";
                    break;
                }
                case OP_ADD: {
                    BINARY_OP(+);
                    break;
                }
                case OP_SUBTRACT: {
                    BINARY_OP(-);
                    break;
                }
                case OP_MULTIPLY: {
                    BINARY_OP(*);
                    break;
                }
                case OP_DIVIDE: {
                    BINARY_OP(/);
                    break;
                }
                case OP_NEGATE: {
                    push(-pop());
                    break;
                }
                case OP_RETURN: {
                    printValue(pop());
                    std::cout << "\n";
                    return INTERPRET_OK;
                }
                default: {
                    std::cout << "Unknown opcode " << instruction << "\n";
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
        }

    #undef READ_BYTE
    #undef READ_24BIT
    #undef READ_CONSTANT
    #undef READ_CONSTANT_LONG
    #undef BINARY_OP
}

interpretResult interpret(const char* source){
    Chunk chunk;
    initChunk(&chunk);

    if(!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    interpretResult result = run();

    freeChunk(&chunk);
    return result;
}