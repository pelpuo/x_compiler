#include "lexer.h"
#include <iostream>

Lexer::Lexer(std::string pstr) {
  Lexer::program_str = pstr;
  BufferStart = &program_str[0];
  BufferPtr = BufferStart;
  line = 1;
}

void Lexer::next(Token &token) {
  while (*BufferPtr && isspace(*BufferPtr)) {
    if (*BufferPtr == '\n') {
      line++;
    }
    ++BufferPtr;
  }

  // Tokenizing identifiers
  if (isalpha(*BufferPtr)) {
    const char *end = BufferPtr + 1;
    while (isalnum(*end))
      ++end;

    std::string value(BufferPtr, end);

    if (value == "return") {
      token.type = TokenType::RETURN;
    } else if (value == "int") {
      token.type = TokenType::INT;
    } else if (value == "void") {
      token.type = TokenType::VOID;
    } else {
      token.type = TokenType::ID;
      token.value = value;
    }
    BufferPtr = end;
    token.line = line;
    return;
  }

  // Tokenizing numbers
  if (isdigit(*BufferPtr)) {
    const char *end = BufferPtr + 1;
    while (isdigit(*end))
      ++end;

    std::string value(BufferPtr, end);
    token.type = TokenType::NUM;
    token.value = value;
    token.line = line;

    BufferPtr = end;
    return;
  }

  // Tokenizing Symbols
  switch (*BufferPtr) {
  case '+':
    token.type = TokenType::PLUS;
    token.line = line;
    BufferPtr++;
    return;
  case '~':
    token.type = TokenType::NEGATION;
    token.line = line;
    BufferPtr++;
    return;
  case '-':
    if (*(BufferPtr + 1) == '-') {
      token.type = TokenType::DECREMENT;
      token.line = line;
      BufferPtr += 2;
    } else {
      token.type = TokenType::MINUS;
      token.line = line;
      BufferPtr++;
    }
    return;
  case '*':
    token.type = TokenType::MUL;
    token.line = line;
    BufferPtr++;
    return;
  case '/':
    token.type = TokenType::DIV;
    token.line = line;
    BufferPtr++;
    return;
  case '=':
    token.type = TokenType::EQUALS;
    token.line = line;
    BufferPtr++;
    return;
  case '(':
    token.type = TokenType::LEFT_PAREN;
    token.line = line;
    BufferPtr++;
    return;
  case ')':
    token.type = TokenType::RIGHT_PAREN;
    token.line = line;
    BufferPtr++;
    return;
  case ';':
    token.type = TokenType::SEMICOLON;
    token.line = line;
    BufferPtr++;
    return;
  case '{':
    token.type = TokenType::LEFT_BRACE;
    token.line = line;
    BufferPtr++;
    return;
  case '%':
    token.type = TokenType::MOD;
    token.line = line;
    BufferPtr++;
    return;
  case '}':
    token.type = TokenType::RIGHT_BRACE;
    token.line = line;
    BufferPtr++;
    return;
  default:
    token.type = TokenType::UNKNOWN;
    break;
  }

  // Locating EOF
  if (*BufferPtr == '\0') {
    token.type = TokenType::EOI;
    return;
  }
  token.value = std::string(BufferPtr, BufferPtr + 1);
  token.type = TokenType::UNKNOWN;
  // cout << "Unknown Token: " << *BufferPtr << endl;
  return;
}