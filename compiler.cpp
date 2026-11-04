#include <format>
#include <iostream>
#include <string_view>
#include <cstring>

#include "include/common.h"
#include "include/compiler.h"
#include "include/scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "include/debug.h"
#endif

struct Parser{
    Token current;
    Token previous;
    bool hadError = false;
    bool panicMode = false;
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

using ParseFn = void (*)(bool canAssign);

struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};

struct Local {
    Token name;
    int depth = 0;
};

struct Compiler {
    Local locals[UINT16_COUNT];
    int localCount = 0;
    int scopeDepth = 0;
};

static Parser parser;
static Compiler* current = nullptr;
static Scanner* scanner = nullptr;
static Chunk* compilingChunk;

static void expression();
static void statement();
static void declaration();
static ParseRule getRule(TokenType type);
static void parsePrecedence(Precedence precedence);
static int identifierConstant(const Token& name);
static int resolveLocal(Compiler* compiler, Token* name);

static Chunk* currentChunk(){
    return compilingChunk;
}

static void errorAt(const Token& token, const char* message){
    if(parser.panicMode) return;
    parser.panicMode = true;
    std::cerr << std::format("[line {}] Error", token.line);

    if(token.type == TOKEN_EOF){
        std::cerr << " at end";
    } else if(token.type == TOKEN_ERROR){

    } else{
        std::cerr << std::format(" at '{}'", std::string_view(token.start, token.length));
    }
    
    std::cerr << std::format(": {}\n", message);
    parser.hadError = true;
}

static void error(const char* message){
    errorAt(parser.previous, message);
}

static void errorAtCurrent(const char* message){
    errorAt(parser.current, message);
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

static bool check(TokenType type){
    return parser.current.type == type;
}

static bool match(TokenType type){
    if(!check(type)) return false;
    advance();
    return true;
}

static void emitByte(std::uint8_t byte){
    currentChunk()->write(byte, parser.previous.line);
}

static void emitBytes(std::uint8_t byte1, std::uint8_t byte2){
    emitByte(byte1);
    emitByte(byte2);
}

static int emitJump(std::uint8_t instruction){
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->count() - 2;
}

static void emitReturn(){
    emitByte(OP_RETURN);
}

static void emitConstant(Value value){
    if(currentChunk()->constantCount() >= (1 << 24)){
        error("Too many constants in one chunk.");
        return;
    }
    currentChunk()->writeConstant(value, parser.previous.line);
}

static void patchJump(int offset){
    int jump = currentChunk()->count() - offset - 2;

    if(jump > UINT16_MAX){
        error("Too much code to jump over");
    }

    currentChunk()->code()[offset] = (jump >> 8) & 0xff;
    currentChunk()->code()[offset + 1] = jump & 0xff;
}

static void initCompiler(Compiler* compiler){
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    current = compiler;
}

static void endCompiler(){
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if(!parser.hadError){
        disassembleChunk(*currentChunk(), "code");
    }
#endif
}

static void beginScope(){
    current->scopeDepth++;
}

static void endScope(){
    current->scopeDepth--;

    while(current->localCount > 0 &&
          current->locals[current->localCount - 1].depth >
          current->scopeDepth){
        emitByte(OP_POP);
        current->localCount--;
    }
}

static void binary(bool canAssign){
    TokenType operatorType = parser.previous.type;
    ParseRule rule = getRule(operatorType);
    parsePrecedence(static_cast<Precedence>(rule.precedence + 1));

    switch(operatorType){
        case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL); break;
        case TOKEN_GREATER:       emitByte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:          emitByte(OP_LESS); break;
        case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
        case TOKEN_PLUS:          emitByte(OP_ADD); break;
        case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
        default: return;
    }
}

static void literal(bool canAssign){
    switch(parser.previous.type){
        case TOKEN_FALSE: emitByte(OP_FALSE); break;
        case TOKEN_NIL:   emitByte(OP_NIL); break;
        case TOKEN_TRUE:  emitByte(OP_TRUE); break;
        default: return;
    }
}

static void grouping(bool canAssign){
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expected ')' after expression.");
}

static void number(bool canAssign){
    double value = std::strtod(parser.previous.start, nullptr);
    emitConstant(numberVal(value));
}

static void or_(bool canAssign){
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
}

static void string(bool canAssign){
    emitConstant(objVal(copyString(parser.previous.start + 1,
                                   parser.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign){
    OpCode getOp, getOpLong, setOp, setOpLong;
    int arg = resolveLocal(current, &name);
    if(arg != -1){
        getOp = OP_GET_LOCAL;
        getOpLong = OP_GET_LOCAL_LONG;
        setOp = OP_SET_LOCAL;
        setOpLong = OP_SET_LOCAL_LONG;
    } else{
        arg = identifierConstant(name);
        getOp = OP_GET_GLOBAL;
        getOpLong = OP_GET_GLOBAL_LONG;
        setOp = OP_SET_GLOBAL;
        setOpLong = OP_SET_GLOBAL_LONG;
    }

    if(canAssign && match(TOKEN_EQUAL)){
        expression();
        currentChunk()->writeWithIndex(setOp, setOpLong, arg, parser.previous.line);
    } else {
        currentChunk()->writeWithIndex(getOp, getOpLong, arg, parser.previous.line);
    }
}

static void variable(bool canAssign){
    namedVariable(parser.previous, canAssign);
}

static void unary(bool canAssign){
    TokenType operatorType = parser.previous.type;

    parsePrecedence(PREC_UNARY);

    switch(operatorType){
        case TOKEN_BANG:  emitByte(OP_NOT); break;
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return;
    }
}

static void parsePrecedence(Precedence precedence){
    advance();
    ParseFn prefixRule = getRule(parser.previous.type).prefix;
    if(prefixRule == nullptr){
        error("Expected expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    while(precedence <= getRule(parser.current.type).precedence){
        advance();
        ParseFn infixRule = getRule(parser.previous.type).infix;
        infixRule(canAssign);
    }

    if(canAssign && match(TOKEN_EQUAL)){
        error("Invalid assignment target.");
    }
}

static int identifierConstant(const Token& name){
    if(currentChunk()->constantCount() >= (1 << 24)) {
        error("Too many constants in one chunk.");
        return 0;
    }
    return currentChunk()->addConstant(objVal(copyString(name.start, name.length)));
}

static bool identifiersEqual(Token* a, Token* b){
    if(a->length != b->length) return false;
    return std::memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler* compiler, Token* name){
    for(int i = compiler->localCount - 1; i >=0; i--){
        Local* local = &compiler->locals[i];
        if(identifiersEqual(name, &local->name)){
            if(local->depth == -1){
                error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

static void addLocal(Token name){
    if(current->localCount == UINT16_COUNT){
        error("too many local variables in function.");
        return;
    }

    Local* local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
}

static void declareVariable(){
    if(current->scopeDepth == 0) return;

    Token* name = &parser.previous;

    for(int i = current->localCount - 1; i >= 0; i--){
        Local* local = &current->locals[i];
        if(local->depth != -1 && local->depth < current->scopeDepth){
            break;
        }

        if(identifiersEqual(name, &local->name)){
            error("Already a variable with this name in this scope.");
        }
    }

    addLocal(*name);
}

static int parseVariable(const char* errorMessage){
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable();
    if(current->scopeDepth > 0) return 0;

    return identifierConstant(parser.previous);
}

static void markInitialized(){
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(int global){
    if(current->scopeDepth > 0){
        markInitialized();
        return;
    }

    currentChunk()->writeWithIndex(OP_DEFINE_GLOBAL, OP_DEFINE_GLOBAL_LONG, global, parser.previous.line);
}

static void and_(bool canAssign){
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    parsePrecedence(PREC_AND);
    
    patchJump(endJump);
}

static ParseRule getRule(TokenType type){
    switch(type){
        case TOKEN_BANG:          return {unary, nullptr, PREC_NONE};
        case TOKEN_BANG_EQUAL:    return {nullptr, binary, PREC_EQUALITY};
        case TOKEN_EQUAL_EQUAL:   return {nullptr, binary, PREC_EQUALITY};
        case TOKEN_GREATER:       return {nullptr, binary, PREC_COMPARISON};
        case TOKEN_GREATER_EQUAL: return {nullptr, binary, PREC_COMPARISON};
        case TOKEN_LESS:          return {nullptr, binary, PREC_COMPARISON};
        case TOKEN_LESS_EQUAL:    return {nullptr, binary, PREC_COMPARISON}; 
        case TOKEN_FALSE:         return {literal, nullptr, PREC_NONE};
        case TOKEN_LEFT_PAREN:    return {grouping, nullptr, PREC_NONE};
        case TOKEN_MINUS:         return {unary,    binary,  PREC_TERM};
        case TOKEN_NIL:           return {literal, nullptr, PREC_NONE};
        case TOKEN_NUMBER:        return {number,   nullptr, PREC_NONE};
        case TOKEN_PLUS:          return {nullptr,  binary,  PREC_TERM};
        case TOKEN_SLASH:         return {nullptr,  binary,  PREC_FACTOR};
        case TOKEN_STAR:          return {nullptr,  binary,  PREC_FACTOR};
        case TOKEN_STRING:        return {string, nullptr, PREC_NONE};
        case TOKEN_TRUE:          return {literal, nullptr, PREC_NONE};
        case TOKEN_IDENTIFIER:    return {variable, nullptr, PREC_NONE};
        case TOKEN_AND:           return {nullptr, and_, PREC_AND};
        case TOKEN_OR:            return {nullptr, or_, PREC_OR};
        default:                  return {nullptr,  nullptr, PREC_NONE};
    }
}

static void expression(){
    parsePrecedence(PREC_ASSIGNMENT);
}

static void block(){
    while(!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)){
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expected '}' after block.");
}

static void varDeclaration(){
    int global = parseVariable("Expected variable name.");

    if(match(TOKEN_EQUAL)){
        expression();
    } else {
        emitByte(OP_NIL);
    }
    
    consume(TOKEN_SEMICOLON, "Expected ';' after variable declaration.");

    defineVariable(global);
}

static void expressionStatement(){
    expression();
    consume(TOKEN_SEMICOLON, "Expected ';' after expression.");
    emitByte(OP_POP);
}

static void ifStatement(){
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if.'");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();

    int elseJump(OP_JUMP);

    patchJump(thenJump);
    emitByte(OP_POP);

    if(match(TOKEN_ELSE)) statement();
    patchJump(elseJump);
}

static void printStatement(){
    consume(TOKEN_LEFT_PAREN, "Expected '(' after print.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expected ')' after value.");
    consume(TOKEN_SEMICOLON, "Expected ';' after value.");
    emitByte(OP_PRINT);
}

static void synchronize(){
    parser.panicMode = false;

    while(parser.current.type != TOKEN_EOF){
        if(parser.previous.type == TOKEN_SEMICOLON) return;
        switch(parser.current.type){
            case TOKEN_CLASS:
            case TOKEN_DEF:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
            default:
                ;
        }

        advance();
    }
}

static void statement(){
    if(match(TOKEN_PRINT)){
        printStatement();
    } else if(match(TOKEN_IF)){
        ifStatement();
    }else if(match(TOKEN_LEFT_BRACE)){
        beginScope();
        block();
        endScope();
    } else{
        expressionStatement();
    }
}

static void declaration(){
    if(match(TOKEN_VAR)){
        varDeclaration();
    } else {
        statement();   
    }

    if(parser.panicMode) synchronize();
}

bool compile(const char* source, Chunk* chunk){
    Scanner s(source);
    scanner = &s;
    Compiler* compiler = new Compiler();
    initCompiler(compiler);
    compilingChunk = chunk;

    parser = Parser{};

    advance();
    while(!match(TOKEN_EOF)){
        declaration();
    }

    scanner = nullptr;
    endCompiler();
    current = nullptr;
    delete compiler;
    return !parser.hadError;
}