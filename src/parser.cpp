#include "parser.h"

void Parser::advance() {
  lexer.next(token);
  if (token.type == TokenType::UNKNOWN) {
    error();
  }
}

bool Parser::expect(TokenType type) {
  if (token.type != type) {
    error();
    return false;
  }
  return true;
}

void Parser::error() {
  cout << "(Parser) Unexpected: " << TokenStr[token.type] << " "
       << token.value.value_or("") << " on line " << token.line << "\n";
  HasError = true;
  exit(1);
}

bool Parser::consume(TokenType type) {
  if (!expect(type))
    return false;
  advance();
  return true;
}

int Parser::getPrecedence(TokenType op) {
  switch (op) {
  case TokenType::INCREMENT:
  case TokenType::DECREMENT:
    return 15;
  case TokenType::LOGICAL_NOT:
    return 14;
  case TokenType::MUL:
  case TokenType::DIV:
  case TokenType::MOD:
    return 13;
  case TokenType::PLUS:
  case TokenType::MINUS:
    return 12;
  case TokenType::LEFT_SHIFT:
  case TokenType::RIGHT_SHIFT:
    return 11;
  case TokenType::LESS_THAN:
  case TokenType::LESS_THAN_EQUAL:
  case TokenType::GREATER_THAN:
  case TokenType::GREATER_THAN_EQUAL:
    return 10;
  case TokenType::EQUAL_EQUAL:
  case TokenType::NOT_EQUAL:
    return 9;
  case TokenType::BITWISE_AND:
    return 8;
  case TokenType::BITWISE_XOR:
    return 7;
  case TokenType::BITWISE_OR:
    return 6;
  case TokenType::LOGICAL_AND:
    return 5;
  case TokenType::LOGICAL_OR:
    return 4;
  case TokenType::QUESTION_MARK:
    return 3;
  case TokenType::EQUALS:
  case TokenType::PLUS_EQUAL:
  case TokenType::MINUS_EQUAL:
  case TokenType::MUL_EQUAL:
  case TokenType::DIV_EQUAL:
  case TokenType::MOD_EQUAL:
  case TokenType::AND_EQUAL:
  case TokenType::OR_EQUAL:
  case TokenType::XOR_EQUAL:
  case TokenType::LEFT_SHIFT_EQUAL:
  case TokenType::RIGHT_SHIFT_EQUAL:
    return 2;
  case TokenType::COMMA:
    return 1;
  default:
    return 0;
  }
}

bool Parser::isBinaryOp(TokenType type) {
  switch (type) {
  case TokenType::PLUS:
  case TokenType::MINUS:
  case TokenType::MUL:
  case TokenType::DIV:
  case TokenType::MOD:
  case TokenType::BITWISE_AND:
  case TokenType::BITWISE_OR:
  case TokenType::BITWISE_XOR:
  case TokenType::LOGICAL_AND:
  case TokenType::LOGICAL_OR:
  case TokenType::LOGICAL_NOT:
  case TokenType::LEFT_SHIFT:
  case TokenType::RIGHT_SHIFT:
  case TokenType::EQUALS:
  case TokenType::EQUAL_EQUAL:
  case TokenType::NOT_EQUAL:
  case TokenType::LESS_THAN:
  case TokenType::LESS_THAN_EQUAL:
  case TokenType::GREATER_THAN:
  case TokenType::GREATER_THAN_EQUAL:

  case TokenType::PLUS_EQUAL:
  case TokenType::MINUS_EQUAL:
  case TokenType::MUL_EQUAL:
  case TokenType::DIV_EQUAL:
  case TokenType::MOD_EQUAL:
  case TokenType::AND_EQUAL:
  case TokenType::OR_EQUAL:
  case TokenType::XOR_EQUAL:
  case TokenType::LEFT_SHIFT_EQUAL:
  case TokenType::RIGHT_SHIFT_EQUAL:
  case TokenType::QUESTION_MARK:
    // case TokenType::COLON:
    return true;
  default:
    return false;
  }
}

bool Parser::isCompoundAssignOp(TokenType type) {
  switch (type) {
  case TokenType::PLUS_EQUAL:
  case TokenType::MINUS_EQUAL:
  case TokenType::MUL_EQUAL:
  case TokenType::DIV_EQUAL:
  case TokenType::MOD_EQUAL:
  case TokenType::AND_EQUAL:
  case TokenType::OR_EQUAL:
  case TokenType::XOR_EQUAL:
  case TokenType::LEFT_SHIFT_EQUAL:
  case TokenType::RIGHT_SHIFT_EQUAL:
    return true;
  default:
    return false;
  }
}

bool Parser::isTypeSpecifier(TokenType type) {
  switch (type) {
  case TokenType::INT:
  case TokenType::LONG:
  case TokenType::UNSIGNED:
  case TokenType::VOID:
    return true;
  default:
    return false;
  }
}

ASTProgram *Parser::parse() {
  ASTProgram *Res = parseProgram();
  expect(TokenType::EOI);
  return Res;
}

ASTProgram *Parser::parseProgram() {
  ASTProgram *program = new ASTProgram();
  FuncDecl *func;

  while (token.type == TokenType::INT) {
    Declaration *decl = parseDeclaration();
    FuncDecl *func = dynamic_cast<FuncDecl *>(decl);
    if (!func) {
      std::cerr << "ERROR: Top-level declarations must be functions\n";
      exit(1);
    }
    if (func->body)
      program->addFunction(std::unique_ptr<FuncDecl>(func));
    else
      program->addPrototype(std::unique_ptr<FuncDecl>(func));
  }

  // Optionally, you can handle cases where there are no functions here.
  // E.g., if we reach this point without parsing any functions, we could throw
  // an error.
  return program;
}

// FuncDecl *Parser::parseFunction() {
//   std::unique_ptr<Block> stmts = std::make_unique<Block>();

//   // Parse the return type of the function (assumed 'int')
//   consume(TokenType::INT);

//   // Parse the function name
//   expect(TokenType::ID);
//   std::string name = token.value.value();
//   consume(TokenType::ID);

//   // Parse the parameters inside parentheses
//   consume(TokenType::LEFT_PAREN);
//   std::vector<std::string> params;

//   // Parse each parameter if available
//   if (token.type != TokenType::RIGHT_PAREN) {
//     do {
//       consume(TokenType::INT); // Assume each param is of type 'int'
//       expect(TokenType::ID);
//       std::string paramName = token.value.value();
//       consume(TokenType::ID);
//       params.push_back(paramName);
//     } while (token.type == TokenType::COMMA && (advance(), true));
//   }

//   consume(TokenType::RIGHT_PAREN);

//   // Parse the function body enclosed in braces
//   consume(TokenType::LEFT_BRACE);
//   BlockItem *nextItem;
//   while (token.type != TokenType::RIGHT_BRACE) {
//     nextItem = parseBlockItem(); // Parse block items (statements,
//     expressions) if (nextItem == nullptr)
//       break; // If no more block items are found, break
//     stmts->addItem(std::unique_ptr<BlockItem>(nextItem));
//   }
//   consume(TokenType::RIGHT_BRACE); // Consume the closing brace

//   // Return a new function declaration object
//   return new FuncDecl(name, std::move(params), std::move(stmts));
// }

BlockItem *Parser::parseBlockItem() {
  // Check if the next token is a type specifier (indicating a declaration)
  if (token.type == TokenType::INT || token.type == TokenType::STATIC ||
      token.type == TokenType::EXTERN || token.type == TokenType::CONST ||
      token.type == TokenType::LONG) {
    return parseDeclaration();
  } else {
    return parseStatement();
  }
}

Declaration *Parser::parseDeclaration() {
  std::unique_ptr<Type> type;
  StorageClass storage;
  TypeQualifier qualifier;
  std::tie(type, storage, qualifier) = parseTypeAndStorageClass();

  expect(TokenType::ID);
  std::string name = token.value.value();
  consume(TokenType::ID);

  if (token.type == TokenType::LEFT_PAREN) {
    // It's a function declaration or definition
    FuncDecl *func = parseFuncDeclOrProto(name, std::move(type));
    func->setStorage(storage);
    return func;
  }

  // Otherwise it's a variable declaration
  VarDecl *var = parseVarDecl(name, std::move(type));
  var->setStorage(storage);
  var->setQualifier(qualifier);
  return var;
}

VarDecl *Parser::parseVarDecl(const std::string &varName,
                              std::unique_ptr<Type> type) {
  std::unique_ptr<Expr> initializer = nullptr;

  // Check for optional initialization (e.g., int x = 10;)
  if (token.type == TokenType::EQUALS) {
    consume(TokenType::EQUALS);
    initializer.reset(parseExpr());
  }

  consume(TokenType::SEMICOLON); // Expect a semicolon at the end
  return new VarDecl(varName, std::move(initializer), std::move(type));
}

FuncDecl *Parser::parseFuncDeclOrProto(const std::string &funcName,
                                       std::unique_ptr<Type> returnType) {
  consume(TokenType::LEFT_PAREN);

  std::vector<std::string> paramNames;
  std::vector<std::unique_ptr<Type>> paramTypes;

  if (token.type != TokenType::RIGHT_PAREN) {
    do {
      auto [paramType, _, __] = parseTypeAndStorageClass();
      expect(TokenType::ID);
      std::string paramName = token.value.value();
      advance(); // consume the ID
      paramNames.push_back(paramName);
      paramTypes.push_back(std::move(paramType));
    } while (token.type == TokenType::COMMA && (advance(), true));
  }

  consume(TokenType::RIGHT_PAREN);

  std::unique_ptr<Type> funcType = std::make_unique<FunctionType>(
      std::move(paramTypes), std::move(returnType));

  // Prototype
  if (token.type == TokenType::SEMICOLON) {
    consume(TokenType::SEMICOLON);
    return new FuncDecl(funcName, std::move(paramNames), nullptr,
                        std::move(funcType));
  }

  // Function definition
  consume(TokenType::LEFT_BRACE);
  std::unique_ptr<Block> body = std::make_unique<Block>();
  BlockItem *nextItem;

  while (token.type != TokenType::RIGHT_BRACE) {
    nextItem = parseBlockItem();
    if (!nextItem)
      break;
    body->addItem(std::unique_ptr<BlockItem>(nextItem));
  }

  consume(TokenType::RIGHT_BRACE);

  return new FuncDecl(funcName, std::move(paramNames), std::move(body),
                      std::move(funcType));
}

// FuncDecl *Parser::parseFuncDecl(const std::string &funcName,
//                                 TypeSpecifier type) {
//   consume(TokenType::LEFT_PAREN);

//   std::vector<std::string> params;
//   if (token.type != TokenType::RIGHT_PAREN) {
//     do {
//       consume(TokenType::INT);
//       expect(TokenType::ID);
//       std::string paramName = token.value.value();
//       consume(TokenType::ID);
//       params.push_back(paramName);
//     } while (token.type == TokenType::COMMA && (advance(), true));
//   }

//   consume(TokenType::RIGHT_PAREN);
//   consume(TokenType::LEFT_BRACE);

//   std::unique_ptr<Block> stmts = std::make_unique<Block>();
//   BlockItem *nextItem;
//   while (token.type != TokenType::RIGHT_BRACE) {
//     nextItem = parseBlockItem();
//     if (nextItem == nullptr)
//       break;
//     stmts->addItem(std::unique_ptr<BlockItem>(nextItem));
//   }
//   consume(TokenType::RIGHT_BRACE);

//   return new FuncDecl(funcName, std::move(params), std::move(stmts));
// }

Stmt *Parser::parseStatement() {
  if (token.type == TokenType::RETURN) {
    consume(TokenType::RETURN);
    Expr *expr = parseExpr();
    std::unique_ptr<Expr> retPtr(expr);
    consume(TokenType::SEMICOLON);
    return new ReturnStmt(std::move(retPtr));
  } else if (token.type == TokenType::ID) {
    Expr *expr = parseExpr();
    consume(TokenType::SEMICOLON);
    return new ExprStmt(std::unique_ptr<Expr>(expr));
  } else if (token.type == TokenType::IF) {
    consume(TokenType::IF);
    consume(TokenType::LEFT_PAREN);
    Expr *expr = parseExpr();
    consume(TokenType::RIGHT_PAREN);
    Stmt *stmt = parseStatement();
    Stmt *elseStmt = nullptr;
    if (token.type == TokenType::ELSE) {
      consume(TokenType::ELSE);
      elseStmt = parseStatement();
    }
    return new IfStmt(std::unique_ptr<Expr>(expr), std::unique_ptr<Stmt>(stmt),
                      std::unique_ptr<Stmt>(elseStmt));
  } else if (token.type == TokenType::LEFT_BRACE) {
    consume(TokenType::LEFT_BRACE);
    std::unique_ptr<Block> block = std::make_unique<Block>();
    BlockItem *nextItem;
    while (token.type != TokenType::RIGHT_BRACE) {
      nextItem = parseBlockItem();
      if (nextItem == nullptr)
        break;
      block->addItem(std::unique_ptr<BlockItem>(nextItem));
    }
    consume(TokenType::RIGHT_BRACE);
    return block.release();
  } else if (token.type == TokenType::WHILE) {
    consume(TokenType::WHILE);
    consume(TokenType::LEFT_PAREN);
    Expr *expr = parseExpr();
    consume(TokenType::RIGHT_PAREN);
    Stmt *stmt = parseStatement();
    return new WhileStmt(std::unique_ptr<Expr>(expr),
                         std::unique_ptr<Stmt>(stmt));
  } else if (token.type == TokenType::FOR) {
    consume(TokenType::FOR);
    consume(TokenType::LEFT_PAREN);

    // Parse init part which can be either a declaration or an expression
    // statement
    std::unique_ptr<BlockItem> init;
    if (token.type == TokenType::INT || token.type == TokenType::STATIC ||
        token.type == TokenType::EXTERN || token.type == TokenType::CONST ||
        token.type == TokenType::LONG) {
      // consume(TokenType::INT); // Consume 'int' keyword
      auto [type, storage, TypeQualifier] = parseTypeAndStorageClass();

      expect(TokenType::ID);

      std::string varName = token.value.value();
      consume(TokenType::ID);
      init.reset(parseVarDecl(varName, std::move(type)));
    } else {
      init.reset(parseExprStmt());
    }

    Expr *cond = parseExpr();
    consume(TokenType::SEMICOLON);
    Expr *inc = parseExpr();
    consume(TokenType::RIGHT_PAREN);
    Stmt *stmt = parseStatement();
    return new ForStmt(std::move(init), std::unique_ptr<Expr>(cond),
                       std::unique_ptr<Expr>(inc), std::unique_ptr<Stmt>(stmt));
  } else if (token.type == TokenType::DO) {
    consume(TokenType::DO);
    Stmt *stmt = parseStatement();
    consume(TokenType::WHILE);
    consume(TokenType::LEFT_PAREN);
    Expr *expr = parseExpr();
    consume(TokenType::RIGHT_PAREN);
    consume(TokenType::SEMICOLON);
    return new DoWhileStmt(std::unique_ptr<Stmt>(stmt),
                           std::unique_ptr<Expr>(expr));
  } else if (token.type == TokenType::BREAK) {
    consume(TokenType::BREAK);
    consume(TokenType::SEMICOLON);
    return new BreakStmt();
  } else if (token.type == TokenType::CONTINUE) {
    consume(TokenType::CONTINUE);
    consume(TokenType::SEMICOLON);
    return new ContinueStmt();
  } else if (token.type == TokenType::SWITCH) {
    consume(TokenType::SWITCH);
    consume(TokenType::LEFT_PAREN);
    Expr *expr = parseExpr();
    consume(TokenType::RIGHT_PAREN);
    consume(TokenType::LEFT_BRACE);

    std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Stmt>>> cases;
    std::unique_ptr<Stmt> defaultCase = nullptr;

    while (token.type != TokenType::RIGHT_BRACE) {
      if (token.type == TokenType::CASE) {
        consume(TokenType::CASE);
        Expr *caseExpr = parseExpr();
        consume(TokenType::COLON);

        // Create a new block to hold multiple statements
        std::unique_ptr<Block> caseBlock = std::make_unique<Block>();

        // Parse multiple statements until another `case`, `default`, or `}`
        while (token.type != TokenType::CASE &&
               token.type != TokenType::DEFAULT &&
               token.type != TokenType::RIGHT_BRACE) {
          Stmt *stmt = parseStatement();
          caseBlock->addItem(std::unique_ptr<BlockItem>(stmt));
        }

        cases.push_back(
            {std::unique_ptr<Expr>(caseExpr), std::move(caseBlock)});
      } else if (token.type == TokenType::DEFAULT) {
        consume(TokenType::DEFAULT);
        consume(TokenType::COLON);

        std::unique_ptr<Block> defaultBlock = std::make_unique<Block>();

        // Collect multiple statements in the default case
        while (token.type != TokenType::CASE &&
               token.type != TokenType::RIGHT_BRACE) {
          Stmt *stmt = parseStatement();
          defaultBlock->addItem(std::unique_ptr<BlockItem>(stmt));
        }

        defaultCase = std::move(defaultBlock);
      }
    }
    consume(TokenType::RIGHT_BRACE);

    SwitchStmt *newSwitch = new SwitchStmt(std::unique_ptr<Expr>(expr));
    for (auto &case_ : cases) {
      newSwitch->addCase(std::move(case_.first), std::move(case_.second));
    }
    if (defaultCase) {
      newSwitch->setDefault(std::move(defaultCase));
    }

    return newSwitch;
  }

  return nullptr;
}

ExprStmt *Parser::parseExprStmt() {
  Expr *expr = parseExpr();
  consume(TokenType::SEMICOLON);
  return new ExprStmt(std::unique_ptr<Expr>(expr));
}

Expr *Parser::parseExpr(int minPrec) {
  Expr *left;

  if (token.type == TokenType::LOGICAL_NOT || token.type == TokenType::MINUS ||
      token.type == TokenType::COMPLEMENT ||
      token.type == TokenType::INCREMENT     // [++ support]
      || token.type == TokenType::DECREMENT) // [++ support]
  {
    TokenType op = token.type;
    int prec = getPrecedence(op);
    advance();
    Expr *right = parseExpr(prec + 1);
    left = new UnaryOp(op, std::unique_ptr<Expr>(right),
                       false); // isPostfix = false // [++ support]
  } else {
    left = parseFactor();
  }

  while (isBinaryOp(token.type) && getPrecedence(token.type) >= minPrec) {
    TokenType op = token.type;
    int prec = getPrecedence(op);
    advance();

    if (op == TokenType::EQUALS) {
      Expr *right = parseExpr(prec);
      left = new Assignment(std::unique_ptr<Expr>(left),
                            std::unique_ptr<Expr>(right));
    } else if (op == TokenType::QUESTION_MARK) {
      Expr *trueExpr = parseExpr();
      consume(TokenType::COLON);
      Expr *falseExpr = parseExpr();
      left = new TernaryOp(std::unique_ptr<Expr>(left),
                           std::unique_ptr<Expr>(trueExpr),
                           std::unique_ptr<Expr>(falseExpr));
    } else if (isCompoundAssignOp(op)) {
      Expr *right = parseExpr(prec);
      left = new CompoundAssignment(op, std::unique_ptr<Expr>(left),
                                    std::unique_ptr<Expr>(right));
    } else {
      Expr *right = parseExpr(prec + 1);
      left = new BinaryOp(op, std::unique_ptr<Expr>(left),
                          std::unique_ptr<Expr>(right));
    }
  }

  return left;
}

Expr *Parser::parseTerm() { return nullptr; }

Expr *Parser::parseFactor() {
  Expr *base;

  // if (token.type == TokenType::NUM) {
  //   string tokentext = token.value.value();
  //   consume(TokenType::NUM);
  //   base = new IntLiteral(stoi(tokentext));
  // } else if (token.type == TokenType::LONG_CONST) {
  //   string tokentext = token.value.value();
  //   consume(TokenType::LONG_CONST);
  //   try {
  //     long longValue = std::stoll(tokentext);
  //     base = new ConstLong(longValue);
  //   } catch (const std::exception &) {
  //     std::cerr << "ERROR: Invalid long constant value\n";
  //     exit(1);
  //   }
  // }
  if (token.type == TokenType::NUM || token.type == TokenType::LONG_CONST) {
    base = parseConstant();
  } else if (token.type == TokenType::LEFT_PAREN) {
    consume(TokenType::LEFT_PAREN);
    base = parseExpr();
    consume(TokenType::RIGHT_PAREN);
  } else if (token.type == TokenType::ID) {
    string name = token.value.value();
    consume(TokenType::ID);

    // Function call
    if (token.type == TokenType::LEFT_PAREN) {
      consume(TokenType::LEFT_PAREN);
      std::unique_ptr<ArgList> args = std::make_unique<ArgList>();
      if (token.type != TokenType::RIGHT_PAREN) {
        do {
          args->addArg(std::unique_ptr<Expr>(parseExpr()));
        } while (token.type == TokenType::COMMA && (advance(), true));
      }
      consume(TokenType::RIGHT_PAREN);
      base = new FuncCall(std::move(name), std::move(args));
    } else {
      base = new Variable(name);
    }
  } else {
    error();
    return nullptr; // [added to prevent control reaching end of non-void]
  }

  // Handle postfix ++ or --
  while (token.type == TokenType::INCREMENT ||
         token.type == TokenType::DECREMENT) { // [++ support]
    TokenType op = token.type;
    advance();
    base = new UnaryOp(op, std::unique_ptr<Expr>(base),
                       true); // isPostfix = true // [++ support]
  }

  return base; // [++ support]
}

// TypeSpecifier Parser::parseType(std::vector<TypeSpecifier> &types) {
//   if (types.empty()) {
//     std::cerr << "ERROR: Missing type specifier\n";
//     exit(1);
//   }

//   if (types.size() == 1 && types[0] == TypeSpecifier::INT) {
//     return TypeSpecifier::INT;
//   }

//   if ((types.size() == 1 && types[0] == TypeSpecifier::LONG) ||
//       (types.size() == 2 &&
//        ((types[0] == TypeSpecifier::INT && types[1] == TypeSpecifier::LONG)
//        ||
//         (types[0] == TypeSpecifier::LONG && types[1] ==
//         TypeSpecifier::INT)))) {
//     return TypeSpecifier::LONG;
//   }

//   std::cerr << "ERROR: Invalid type specifier combination\n";
//   exit(1);
// }

std::tuple<std::unique_ptr<Type>, StorageClass, TypeQualifier>
Parser::parseTypeAndStorageClass() {
  std::vector<TokenType> specifiers;

  // Collect all valid type/storage/qualifier specifiers
  while (token.type == TokenType::STATIC || token.type == TokenType::EXTERN ||
         token.type == TokenType::CONST || token.type == TokenType::INT ||
         token.type == TokenType::LONG) {
    specifiers.push_back(token.type);
    advance();
  }

  bool seenConst = false;
  bool seenInt = false;
  bool seenLong = false;
  StorageClass storage = StorageClass::NONE;

  for (TokenType spec : specifiers) {
    switch (spec) {
    case TokenType::INT:
      if (seenInt) {
        std::cerr << "ERROR: Duplicate 'int' specifier\n";
        exit(1);
      }
      seenInt = true;
      break;
    case TokenType::LONG:
      if (seenLong) {
        std::cerr << "ERROR: Duplicate 'long' specifier\n";
        exit(1);
      }
      seenLong = true;
      break;
    case TokenType::CONST:
      if (seenConst) {
        std::cerr << "ERROR: Duplicate 'const' qualifier\n";
        exit(1);
      }
      seenConst = true;
      break;
    case TokenType::STATIC:
      if (storage != StorageClass::NONE) {
        std::cerr << "ERROR: Multiple storage classes\n";
        exit(1);
      }
      storage = StorageClass::STATIC;
      break;
    case TokenType::EXTERN:
      if (storage != StorageClass::NONE) {
        std::cerr << "ERROR: Multiple storage classes\n";
        exit(1);
      }
      storage = StorageClass::EXTERN;
      break;
    default:
      error();
    }
  }

  // Determine the base type
  std::unique_ptr<Type> baseType;

  if (seenLong) {
    baseType = std::make_unique<LongType>();
  } else if (seenInt && (!seenLong)) {
    // default type is int if nothing is specified
    baseType = std::make_unique<IntType>();
  } else {
    std::cerr << "ERROR: Invalid or unsupported type combination\n";
    exit(1);
  }

  TypeQualifier qualifier =
      seenConst ? TypeQualifier::CONST : TypeQualifier::NONE;

  return {std::move(baseType), storage, qualifier};
}

Expr *Parser::parseConstant() {
  if (token.type == TokenType::NUM) {
    std::string tokentext = token.value.value();

    uint64_t v;
    try {
      v = std::stoull(tokentext);
    } catch (const std::exception &) {
      std::cerr << "ERROR: Invalid constant value\n";
      exit(1);
    }

    if (v > INT64_MAX) {
      std::cerr
          << "ERROR: Constant is too large to represent as an int or long\n";
      exit(1);
    }

    // consume(TokenType::NUM);
    consume(token.type); // Consume either NUM or LONG_CONST

    if (v <= INT32_MAX) {
      return new ConstInt(static_cast<int>(v));
    } else {
      return new ConstLong(static_cast<long long>(v));
    }

  } else if (token.type == TokenType::LONG_CONST) {
    std::string tokentext = token.value.value();
    long long v;
    try {
      v = std::stoll(tokentext);
    } catch (const std::exception &) {
      std::cerr << "ERROR: Invalid long constant value\n";
      exit(1);
    }
    consume(TokenType::LONG_CONST); // Consume LONG_CONST
    return new ConstLong(v);
  } else {
    error();
    return nullptr; // Prevent control reaching end of non-void
  }
}

Expr *Parser::parseCast() {
  if (token.type == TokenType::LEFT_PAREN) {
    consume(TokenType::LEFT_PAREN);

    if (isTypeSpecifier(token.type)) {
      auto [type, storage, qual] = parseTypeAndStorageClass();
      consume(TokenType::RIGHT_PAREN);
      Expr *expr = parseExpr(getPrecedence(TokenType::LOGICAL_NOT));
      return new Cast(std::unique_ptr<Expr>(expr), std::move(type));
    }

    // If not a type, it's a parenthesized expression
    Expr *inner = parseExpr();
    consume(TokenType::RIGHT_PAREN);
    return inner;
  }

  return parseFactor();
}

Parser::Parser(Lexer &lex) : lexer(lex), HasError(false) { advance(); }