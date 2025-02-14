
#pragma once

#include <string>
#include <optional>
#include <vector>

using namespace std;

typedef enum{
    EOI,
    SEMICOLON,
    PLUS,
    MINUS,
    MUL,
    DIV,
    MOD,
    EQUALS,
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    NUM,
    ID,
    RETURN,
    INT,
    VOID,
    DECREMENT,
    NEGATION,
    UNKNOWN,
}TokenType;

static string TokenStr[] = {
    "EOI",
    "SEMICOLON",
    "PLUS",
    "MINUS",
    "MUL",
    "DIV",
    "MOD",
    "EQUALS",
    "LEFT_PAREN",
    "RIGHT_PAREN",
    "LEFT_BRACE",
    "RIGHT_BRACE",
    "NUM",
    "ID",
    "RETURN",
    "INT",
    "VOID",
    "DECREMENT",
    "NEGATION",
    "UNKNOWN",
};

typedef struct{
    TokenType type;
    std::optional<std::string> value;
    int line;
}Token;

class Lexer{
    public:
        const char* BufferStart;
        const char* BufferPtr;
        int line;
        std::string program_str;

        Lexer(std::string program_str);
        void next(Token &token);
        // void formToken(Token &token, const char *tokenEnd, TokenType type);
};
std::vector<Token> tokenize(const std::string& str);