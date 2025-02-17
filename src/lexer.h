
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
    COMPLEMENT,
    UNKNOWN,
    
    BITWISE_AND,
    BITWISE_OR,
    BITWISE_XOR,
    LEFT_SHIFT,
    RIGHT_SHIFT,

    LOGICAL_AND,
    LOGICAL_OR,
    LOGICAL_NOT,
    EQUAL_EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    GREATER_THAN,
    LESS_THAN_EQUAL,
    GREATER_THAN_EQUAL,

    PLUS_EQUAL,
    MINUS_EQUAL,
    MUL_EQUAL,
    DIV_EQUAL,
    MOD_EQUAL,

    AND_EQUAL,
    OR_EQUAL,
    XOR_EQUAL,
    LEFT_SHIFT_EQUAL,
    RIGHT_SHIFT_EQUAL,

    IF,
    ELSE,
    QUESTION_MARK,
    COLON,
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
    "COMPLEMENT",
    "UNKNOWN",
    "BITWISE_AND",
    "BITWISE_OR",
    "BITWISE_XOR",
    "LEFT_SHIFT",
    "RIGHT_SHIFT",
    "LOGICAL_AND",
    "LOGICAL_OR",
    "LOGICAL_NOT",

    "EQUAL_EQUAL",
    "NOT_EQUAL",
    "LESS_THAN",
    "GREATER_THAN",
    "LESS_THAN_EQUAL",
    "GREATER_THAN_EQUAL",

    "PLUS_EQUAL",
    "MINUS_EQUAL",
    "MUL_EQUAL",
    "DIV_EQUAL",
    "MOD_EQUAL",

    "AND_EQUAL",
    "OR_EQUAL",
    "XOR_EQUAL",
    "LEFT_SHIFT_EQUAL",
    "RIGHT_SHIFT_EQUAL",

    "IF",
    "ELSE",
    "QUESTION_MARK",
    "COLON",
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