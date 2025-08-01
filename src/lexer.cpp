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
    } else if (value == "long") {
      token.type = TokenType::LONG;
    } else if (value == "double") {
      token.type = TokenType::DOUBLE;
    } else if (value == "signed") {
      token.type = TokenType::SIGNED;
    } else if (value == "unsigned") {
      token.type = TokenType::UNSIGNED;
    } else if (value == "const") {
      token.type = TokenType::CONST;
    } else if (value == "unsigned") {
      token.type = TokenType::UNSIGNED;
    } else if (value == "void") {
      token.type = TokenType::VOID;
    } else if (value == "if") {
      token.type = TokenType::IF;
    } else if (value == "else") {
      token.type = TokenType::ELSE;
    } else if (value == "while") {
      token.type = TokenType::WHILE;
    } else if (value == "for") {
      token.type = TokenType::FOR;
    } else if (value == "do") {
      token.type = TokenType::DO;
    } else if (value == "break") {
      token.type = TokenType::BREAK;
    } else if (value == "continue") {
      token.type = TokenType::CONTINUE;
    } else if (value == "switch") {
      token.type = TokenType::SWITCH;
    } else if (value == "case") {
      token.type = TokenType::CASE;
    } else if (value == "default") {
      token.type = TokenType::DEFAULT;
    } else if (value == "extern") {
      token.type = TokenType::EXTERN;
    } else if (value == "static") {
      token.type = TokenType::STATIC;
    } else {
      token.type = TokenType::ID;
      token.value = value;
    }
    BufferPtr = end;
    token.line = line;
    return;
  }

  // Tokenizing numbers (int, float, long, unsigned)
  if (isdigit(*BufferPtr) || (*BufferPtr == '.' && isdigit(*(BufferPtr + 1)))) {
    const char *start = BufferPtr;
    const char *p = BufferPtr;

    bool isFloat = false;

    // Integer part
    while (isdigit(*p)) ++p;

    // Decimal part
    if (*p == '.') {
      isFloat = true;
      ++p;
      while (isdigit(*p)) ++p;
    }

    // Scientific notation
    if (*p == 'e' || *p == 'E') {
      isFloat = true;
      ++p;
      if (*p == '+' || *p == '-') ++p;
      if (!isdigit(*p)) {
        std::cerr << "Invalid exponent in floating-point constant\n";
        exit(1);
      }
      while (isdigit(*p)) ++p;
    }

    std::string value(start, p);

    // Handle suffixes (only for integers)
    std::string suffix;
    if (!isFloat && (*p == 'u' || *p == 'U' || *p == 'l' || *p == 'L')) {
      suffix += tolower(*p);
      ++p;
      if (*p == 'u' || *p == 'U' || *p == 'l' || *p == 'L') {
        suffix += tolower(*p);
        ++p;
      }

      if (suffix == "l")
        token.type = TokenType::LONG_CONST;
      else if (suffix == "ul" || suffix == "lu")
        token.type = TokenType::UNSIGNED_LONG_CONST;
      else if (suffix == "u")
        token.type = TokenType::UNSIGNED_INT_CONST;
      else {
        std::cerr << "Invalid integer suffix: " << suffix << "\n";
        exit(1);
      }
    } else if (isFloat) {
      token.type = TokenType::FLOAT_CONST;
    } else {
      token.type = TokenType::NUM; // default: signed int
    }

    token.value = value;
    token.line = line;
    BufferPtr = p;
    return;
  }


  // Tokenizing Symbols
  switch (*BufferPtr) {
  case '+':
    if (*(BufferPtr + 1) == '+') {
      token.type = TokenType::INCREMENT;
      token.line = line;
      BufferPtr += 2;
    } else if (*(BufferPtr + 1) == '=') {
      token.type = TokenType::PLUS_EQUAL;
      token.line = line;
      BufferPtr += 2;
    } else {
      token.type = TokenType::PLUS;
      token.line = line;
      BufferPtr++;
    }
    return;
  case '~':
    token.type = TokenType::COMPLEMENT;
    token.line = line;
    BufferPtr++;
    return;
  case '-':
    if (*(BufferPtr + 1) == '-') {
      token.type = TokenType::DECREMENT;
      token.line = line;
      BufferPtr += 2;
    } else if (*(BufferPtr + 1) == '=') {
      token.type = TokenType::MINUS_EQUAL;
      token.line = line;
      BufferPtr += 2;
    } else {
      token.type = TokenType::MINUS;
      token.line = line;
      BufferPtr++;
    }
    return;
  case '*':
    if (*(BufferPtr + 1) == '=') {
      token.type = TokenType::MUL_EQUAL;
      token.line = line;
      BufferPtr += 2;
    } else {
      token.type = TokenType::MUL;
      token.line = line;
      BufferPtr++;
    }
    return;
  case '/':
    if (*(BufferPtr + 1) == '=') {
      token.type = TokenType::DIV_EQUAL;
      token.line = line;
      BufferPtr += 2;
    } else if (*(BufferPtr + 1) == '/') {
      while (*BufferPtr != '\n') {
        BufferPtr++;
      }
      next(token);
    } else if (*(BufferPtr + 1) == '*') {
      BufferPtr += 2;
      while (*BufferPtr != '*' && *(BufferPtr + 1) != '/') {
        if (*BufferPtr == '\n') {
          line++;
        }
        BufferPtr++;
      }
      BufferPtr += 2;
      next(token);
    } else {
      token.type = TokenType::DIV;
      token.line = line;
      BufferPtr++;
    }
    return;
  case '=':
    if (*(BufferPtr + 1) == '=') {
      token.type = TokenType::EQUAL_EQUAL;
      token.line = line;
      BufferPtr += 2;
    } else {
      token.type = TokenType::EQUALS;
      token.line = line;
      BufferPtr++;
    }
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
  case '&':
    if (*(BufferPtr + 1) == '&') {
      token.type = TokenType::LOGICAL_AND;
      token.line = line;
      BufferPtr += 2;
    } else if (*(BufferPtr + 1) == '=') {
      token.type = TokenType::AND_EQUAL;
      token.line = line;
      BufferPtr += 2;
    } else {
      token.type = TokenType::BITWISE_AND;
      token.line = line;
      BufferPtr++;
    }
    return;
  case '|':
    if (*(BufferPtr + 1) == '|') {
      token.type = TokenType::LOGICAL_OR;
      token.line = line;
      BufferPtr += 2;
    } else if (*(BufferPtr + 1) == '=') {
      token.type = TokenType::OR_EQUAL;
      token.line = line;
      BufferPtr += 2;
    } else {
      token.type = TokenType::BITWISE_OR;
      token.line = line;
      BufferPtr++;
    }
    return;
  case '!':
    if (*(BufferPtr + 1) == '=') {
      token.type = TokenType::NOT_EQUAL;
      token.line = line;
      BufferPtr += 2;
    } else {
      token.type = TokenType::LOGICAL_NOT;
      token.line = line;
      BufferPtr++;
    }
    return;
  case '^':
    if (*(BufferPtr + 1) == '=') {
      token.type = TokenType::XOR_EQUAL;
      token.line = line;
      BufferPtr += 2;
    } else {
      token.type = TokenType::BITWISE_XOR;
      token.line = line;
      BufferPtr++;
    }
  case '<':
    if (*(BufferPtr + 1) == '<') {
      if (*(BufferPtr + 2) == '=') {
        token.type = TokenType::LEFT_SHIFT_EQUAL;
        token.line = line;
        BufferPtr += 3;
      } else {
        token.type = TokenType::LEFT_SHIFT;
        token.line = line;
        BufferPtr += 2;
      }
    } else if (*(BufferPtr + 1) == '=') {
      token.type = TokenType::LESS_THAN_EQUAL;
      token.line = line;
      BufferPtr += 2;
    } else {
      token.type = TokenType::LESS_THAN;
      token.line = line;
      BufferPtr++;
    }
    return;
  case '>':
    if (*(BufferPtr + 1) == '>') {
      if (*(BufferPtr + 2) == '=') {
        token.type = TokenType::RIGHT_SHIFT_EQUAL;
        token.line = line;
        BufferPtr += 3;
      } else {
        token.type = TokenType::RIGHT_SHIFT;
        token.line = line;
        BufferPtr += 2;
      }
    } else if (*(BufferPtr + 1) == '=') {
      token.type = TokenType::GREATER_THAN_EQUAL;
      token.line = line;
      BufferPtr += 2;
    } else {
      token.type = TokenType::GREATER_THAN;
      token.line = line;
      BufferPtr++;
    }
    return;
  case '?':
    token.type = TokenType::QUESTION_MARK;
    token.line = line;
    BufferPtr++;
    return;
  case ':':
    token.type = TokenType::COLON;
    token.line = line;
    BufferPtr++;
    return;
  case ',':
    token.type = TokenType::COMMA;
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