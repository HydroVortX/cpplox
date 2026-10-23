#pragma once

enum TokenType {
    // Single-character tokens.
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,
    // One or two character tokens.
    TOKEN_PLUS_PLUS, TOKEN_PLUS_EQUAL,
    TOKEN_MINUS_MINUS, TOKEN_MINUS_EQUAL,
    TOKEN_SLASH_EQUAL, TOKEN_STAR_EQUAL,
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,
    // Literals.
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
    // Keywords.
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_DEF, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

    TOKEN_ERROR, TOKEN_EOF
};

struct Token {
    TokenType type = TOKEN_EOF;
    const char* start = nullptr;
    int length = 0;
    int line = 0;
};

class Scanner {
    public:
        explicit Scanner(const char* source)
            : start_(source), current_(source), line_(1) {}
        
            Token scanToken();

    private:
        bool isAtEnd() const;
        char advance();
        char peek() const;
        char peekNext() const;
        bool match(char expected);

        Token makeToken(TokenType type) const;
        Token errorToken(const char* message) const;
        void skipWhitespace();
        TokenType checkKeyword(int start, int length, const char* rest, TokenType type) const;
        TokenType identifierType() const;
        Token identifier();
        Token number();
        Token scanString();

        const char* start_;
        const char* current_;
        int line_;
};