#include <iostream>
#include <format>
#include <cstdarg>
#include <cstring>

#include "include/common.h"
#include "include/vm.h"
#include "include/debug.h"
#include "include/memory.h"
#include "include/compiler.h"
#include "include/object.h"

VM vm;

static void resetStack(){
    vm.stackTop = vm.stack;
    vm.stackCount = 0;
}

template <typename... Args>
static void runtimeError(std::format_string<Args...> fmt, Args&&... args){
    std::cerr << std::format(fmt, std::forward<Args>(args)...) << '\n';
    std::size_t instruction = static_cast<std::size_t>(vm.ip - vm.chunk->code) - 1;

    int line = getLine(vm.chunk, static_cast<int>(instruction));
    std::cerr << std::format("[line {}] in script\n", line);
    resetStack();
}

void initVM(){
    resetStack();
    vm.objects = nullptr;
}

void freeVM(){
    freeObjects();
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

static Value peek(int distance){
    return vm.stackTop[-1 - distance];
}

static bool isFalsey(Value value){
    return isNil(value) || (isBool(value) && !asBool(value));
}

static void concatenate(){
    ObjString* b = asString(pop());
    ObjString* a = asString(pop());

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    std::memcpy(chars, a->chars, a->length);
    std::memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    push(objVal(result));
}

static interpretResult run(){
    #define READ_BYTE() (*vm.ip++)
    #define READ_24BIT() (vm.ip += 3, ((vm.ip[-3]) << 16| (vm.ip[-2]) << 8 | vm.ip[-1]))
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    #define READ_CONSTANT_LONG() (vm.chunk->constants.values[READ_24BIT()])
    #define BINARY_OP(ValueType, op) \
        do { \
            if(!isNumber(peek(0)) || !isNumber(peek(1))){ \
                runtimeError("Operands must be numbers."); \
                return INTERPRET_RUNTIME_ERROR; \
            } \
            double b = asNumber(pop()); \
            double a = asNumber(pop()); \
            push(ValueType(a op b)); \
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
            std::uint8_t instruction;
            switch (instruction = READ_BYTE()) {
                case OP_CONSTANT_LONG: {
                    Value constant = READ_CONSTANT_LONG();
                    push(constant);
                    break;
                }
                case OP_CONSTANT: {
                    Value constant = READ_CONSTANT();
                    push(constant);
                    break;
                }
                case OP_NIL: {
                    push(nilVal()); 
                    break;
                }
                case OP_TRUE: {
                    push(boolVal(true)); 
                    break;
                }
                case OP_FALSE: {
                    push(boolVal(false));
                    break;
                }
                case OP_EQUAL: {
                    Value b = pop();
                    Value a = pop();
                    push(boolVal(valuesEqual(a, b)));
                    break;
                }
                case OP_GREATER: {
                    BINARY_OP(boolVal, >); 
                    break;
                }
                case OP_LESS: {
                    BINARY_OP(boolVal, <);
                    break;
                }
                case OP_ADD: {
                    if(isString(peek(0)) && isString(peek(1))){
                        concatenate();
                    } else if(isNumber(peek(0)) && isNumber(peek(1))){
                        BINARY_OP(numberVal, +);
                    } else{
                        runtimeError("Operands must be two numbers or two strings.");
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    break;
                }
                case OP_SUBTRACT: {
                    BINARY_OP(numberVal, -);
                    break;
                }
                case OP_MULTIPLY: {
                    BINARY_OP(numberVal, *);
                    break;
                }
                case OP_DIVIDE: {
                    BINARY_OP(numberVal, /);
                    break;
                }
                case OP_NOT:
                    push(boolVal(isFalsey(pop())));
                    break;
                case OP_NEGATE: {
                    if(!isNumber(peek(0))) {
                        runtimeError("Operand must be a number.");
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    push(numberVal(-asNumber(pop())));
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