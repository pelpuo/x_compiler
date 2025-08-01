
#pragma once

#include <string>
#include <optional>
#include <vector>

using namespace std;

enum class StorageClass {
  NONE,
  STATIC,
  EXTERN
};

enum class TypeSpecifier {
  INT,
  LONG,
  // Add these if needed later: UNSIGNED, LONG, etc.
};

enum class TypeQualifier {
  NONE,
  CONST
};

static std::string TypeSpecifierString[] = {
  "int",
  "long",
  // Add these if needed later: UNSIGNED, LONG, etc.
};

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
    LONG,
    LONG_CONST,
    UNSIGNED_LONG_CONST,
    UNSIGNED_INT_CONST,
    UNSIGNED,
    SIGNED,
    VOID,
    INCREMENT,
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

    COMMA,

    WHILE,
    FOR,
    DO,
    BREAK,
    CONTINUE,

    SWITCH,
    CASE,
    DEFAULT,

    EXTERN,
    STATIC,
    CONST
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
    "LONG",
    "LONG_CONST",
    "UNSIGNED_LONG_CONST",
    "UNSIGNED_INT_CONST",
    "UNSIGNED",
    "SIGNED",
    "VOID",
    "INCREMENT",
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

    "COMMA",

    "WHILE",
    "FOR",
    "DO",
    "BREAK",
    "CONTINUE",

    "SWITCH",
    "CASE",
    "DEFAULT",

    "EXTERN",
    "STATIC",
    "CONST"
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