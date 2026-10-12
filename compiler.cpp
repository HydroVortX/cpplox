#include <format>
#include <iostream>
#include <string_view>

#include "include/common.h"
#include "include/compiler.h"
#include "include/scanner.h"

struct Parser{
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
};

static Parser parser;
static Scanner* scanner = nullptr;
Chunk* compilingChunk;

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

static void endCompiler(){
    emitReturn();
}

static void expression(){
    
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