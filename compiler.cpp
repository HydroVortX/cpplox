#include <format>
#include <iostream>
#include <string_view>

#include "include/common.h"
#include "include/compiler.h"
#include "include/scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "include/debug.h"
#endif

struct Parser{
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
};

enum Precedence {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
};

using ParseFn = void (*)();

struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};

static Parser parser;
static Scanner* scanner = nullptr;
static Chunk* compilingChunk;

static Chunk* currentChunk(){
    return compilingChunk;
}

static void errorAt(Token* token, const char* message){
    if(parser.panicMode) return;
    parser.panicMode = true;
    std::cerr << std::format("[line {}] Error", token->line);

    if(token->type == TOKEN_EOF){
        std::cerr << " at end";
    } else if(token->type == TOKEN_ERROR){

    } else{
        std::cerr << std::format(" at '{}'", std::string_view(token->start, token->length));
    }
    
    std::cerr << std::format(": {}\n", message);
    parser.hadError = true;
}

static void error(const char* message){
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char* message){
    errorAt(&parser.current, message);
}

static void advance(){
    parser.previous = parser.current;

    for(;;){
        parser.current = scanner->scanToken();
        if(parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char* message){
    if(parser.current.type == type){
        advance();
        return;
    }

    errorAtCurrent(message);
}

static void emitByte(uint8_t byte){
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2){
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn(){
    emitByte(OP_RETURN);
}

static void emitConstant(Value value){
    if(currentChunk()->constants.count >= (1 << 24)){
        error("Too many constants in one chunk.");
        return;
    }
    writeConstant(currentChunk(), value, parser.previous.line);
}

static void endCompiler(){
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if(!parser.hadError){
        disassembleChunk(currentChunk(), "code");
    }
#endif
}

static void expression();
static ParseRule getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static void binary(){
    TokenType operatorType = parser.previous.type;
    ParseRule rule = getRule(operatorType);
    parsePrecedence(static_cast<Precedence>(rule.precedence + 1));

    switch(operatorType){
        case TOKEN_PLUS: emitByte(OP_ADD); break;
        case TOKEN_MINUS: emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR: emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH: emitByte(OP_DIVIDE); break;
        default: return;
    }
}

static void grouping(){
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

static void number(){
    double value = std::strtod(parser.previous.start, nullptr);
    emitConstant(value);
}

static void unary(){
    TokenType operatorType = parser.previous.type;

    parsePrecedence(PREC_UNARY);

    switch(operatorType){
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return;
    }
}

static void parsePrecedence(Precedence precedence){
    advance();
    ParseFn prefixRule = getRule(parser.previous.type).prefix;
    if(prefixRule == nullptr){
        error("Expect expression.");
        return;
    }

    prefixRule();

    while(precedence <= getRule(parser.current.type).precedence){
        advance();
        ParseFn infixRule = getRule(parser.previous.type).infix;
        infixRule();
    }
}

static ParseRule getRule(TokenType type){
    switch(type){
        case TOKEN_LEFT_PAREN: return {grouping, nullptr, PREC_NONE};
        case TOKEN_MINUS:      return {unary,    binary,  PREC_TERM};
        case TOKEN_PLUS:       return {nullptr,  binary,  PREC_TERM};
        case TOKEN_SLASH:      return {nullptr,  binary,  PREC_FACTOR};
        case TOKEN_STAR:       return {nullptr,  binary,  PREC_FACTOR};
        case TOKEN_NUMBER:     return {number,   nullptr, PREC_NONE};
        default:               return {nullptr,  nullptr, PREC_NONE};
    }
}


static void expression(){
    parsePrecedence(PREC_ASSIGNMENT);
}

bool compile(const char* source, Chunk* chunk){
    Scanner s(source);
    scanner = &s;
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    advance();
    expression();
    consume(TOKEN_EOF, "Expected end of expression.");

    scanner = nullptr;
    endCompiler();
    return !parser.hadError;
}