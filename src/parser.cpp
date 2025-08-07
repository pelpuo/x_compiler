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
  case TokenType::DOUBLE:
  case TokenType::SIGNED:
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

// ASTProgram *Parser::parseProgram() {
//   ASTProgram *program = new ASTProgram();
//   FuncDecl *func;

//   while (isTypeSpecifier(token.type) || token.type == TokenType::STATIC ||
//          token.type == TokenType::EXTERN || token.type == TokenType::CONST) {
//     Declaration *decl = parseDeclaration();
//     FuncDecl *func = dynamic_cast<FuncDecl *>(decl);
//     if (!func) {
//       std::cerr << "ERROR: Top-level declarations must be functions\n";
//       exit(1);
//     }
//     if (func->body)
//       program->addFunction(std::unique_ptr<FuncDecl>(func));
//     else
//       program->addPrototype(std::unique_ptr<FuncDecl>(func));
//   }

//   return program;
// }

ASTProgram *Parser::parseProgram() {
  ASTProgram *program = new ASTProgram();
  FuncDecl *func;

  while (isTypeSpecifier(token.type) || token.type == TokenType::STATIC ||
         token.type == TokenType::EXTERN || token.type == TokenType::CONST) {
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

  return program;
}

BlockItem *Parser::parseBlockItem() {
  // Check if the next token is a type specifier (indicating a declaration)
  if (token.type == TokenType::INT || token.type == TokenType::STATIC ||
      token.type == TokenType::EXTERN || token.type == TokenType::CONST ||
      token.type == TokenType::SIGNED || token.type == TokenType::UNSIGNED ||
      token.type == TokenType::LONG || token.type == TokenType::DOUBLE) {
    return parseDeclaration();
  } else {
    return parseStatement();
  }
}

std::unique_ptr<Declarator> Parser::parseDeclarator() {
  if (token.type == TokenType::MUL) {
    consume(TokenType::MUL);
    auto inner = parseDeclarator();
    return std::make_unique<Declarator>(
        Declarator{Declarator::POINTER, "", std::move(inner), {}});
  }

  if (token.type == TokenType::LEFT_PAREN) {
    consume(TokenType::LEFT_PAREN);
    auto inner = parseDeclarator();
    consume(TokenType::RIGHT_PAREN);

    if (token.type == TokenType::LEFT_PAREN) {
      return parseFunctionDeclarator(std::move(inner));
    }

    return inner;
  }

  if (token.type == TokenType::ID) {
    std::string name = token.value.value();
    consume(TokenType::ID);

    if (token.type == TokenType::LEFT_PAREN) {
      auto base = std::make_unique<Declarator>(
          Declarator{Declarator::IDENT, name, nullptr, {}});
      return parseFunctionDeclarator(std::move(base));
    }

    return std::make_unique<Declarator>(
        Declarator{Declarator::IDENT, name, nullptr, {}});
  }

  error();
  return nullptr;
}

std::unique_ptr<Declarator>
Parser::parseFunctionDeclarator(std::unique_ptr<Declarator> inner) {
  consume(TokenType::LEFT_PAREN);

  std::vector<std::pair<std::unique_ptr<Type>, std::string>> params;

  if (token.type != TokenType::RIGHT_PAREN) {
    do {
      auto [type, _, __] = parseTypeAndStorageClass();
      expect(TokenType::ID);
      std::string paramName = token.value.value();
      consume(TokenType::ID);
      params.emplace_back(std::move(type), paramName);
    } while (token.type == TokenType::COMMA && (advance(), true));
  }

  consume(TokenType::RIGHT_PAREN);
  return std::make_unique<Declarator>(Declarator{
      Declarator::FUNCTION, "", std::move(inner), std::move(params)});
}

std::tuple<std::string, std::unique_ptr<Type>, std::vector<std::string>>
Parser::processDeclarator(std::unique_ptr<Declarator> d,
                          std::unique_ptr<Type> base) {
  switch (d->kind) {
  case Declarator::IDENT:
    return std::make_tuple(d->ident, std::move(base),
                           std::vector<std::string>{});

  case Declarator::POINTER: {
    auto ptrType = std::make_unique<PointerType>(std::move(base));
    return processDeclarator(std::move(d->inner), std::move(ptrType));
  }

  case Declarator::FUNCTION: {
    if (d->inner->kind != Declarator::IDENT) {
      std::cerr << "ERROR: Functions returning functions or function pointers "
                   "not supported\n";
      exit(1);
    }
    std::string name = d->inner->ident;
    std::vector<std::string> paramNames;
    std::vector<std::unique_ptr<Type>> paramTypes;

    for (auto &[ptype, pname] : d->params) {
      paramNames.push_back(pname);
      paramTypes.push_back(std::move(ptype));
    }

    auto funcType =
        std::make_unique<FunctionType>(std::move(paramTypes), std::move(base));
    return {name, std::move(funcType), std::move(paramNames)};
  }
  }

  error();
  return std::make_tuple("", nullptr, std::vector<std::string>{});
}

Declaration *Parser::parseDeclaration() {
  std::unique_ptr<Type> type;
  StorageClass storage;
  TypeQualifier qualifier;
  std::tie(type, storage, qualifier) = parseTypeAndStorageClass();

  // expect(TokenType::ID);
  // std::string name = token.value.value();
  // consume(TokenType::ID);

  auto declarator = parseDeclarator();
  auto [name, fullType, paramNames] =
      processDeclarator(std::move(declarator), std::move(type));

  if (fullType->isFunctionType()) {
    // Use existing parseFuncDeclOrProto but pass fullType and paramNames
    FuncDecl *func =
        parseFuncDeclOrProto(name, std::move(fullType), std::move(paramNames));
    func->setStorage(storage);
    return func;
  } else {
    // Variable
    return parseVarDecl(name, std::move(fullType));
  }

  // if (token.type == TokenType::LEFT_PAREN) {
  //   // It's a function declaration or definition
  //   FuncDecl *func = parseFuncDeclOrProto(name, std::move(type));
  //   func->setStorage(storage);
  //   return func;
  // }

  // // Otherwise it's a variable declaration
  // VarDecl *var = parseVarDecl(name, std::move(type));
  // var->setStorage(storage);
  // var->setQualifier(qualifier);
  // return var;
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

// FuncDecl *Parser::parseFuncDeclOrProto(const std::string &funcName,
//                                        std::unique_ptr<Type> returnType)
// FuncDecl *Parser::parseFuncDeclOrProto(std::string funcName,
//                                        std::unique_ptr<Type> returnType,
//                                        std::vector<std::string> paramNames) {
//   consume(TokenType::LEFT_PAREN);

//   // std::vector<std::string> paramNames;
//   std::vector<std::unique_ptr<Type>> paramTypes;

//   if (token.type != TokenType::RIGHT_PAREN) {
//     do {
//       auto [paramType, _, __] = parseTypeAndStorageClass();
//       expect(TokenType::ID);
//       std::string paramName = token.value.value();
//       advance(); // consume the ID
//       paramNames.push_back(paramName);
//       paramTypes.push_back(std::move(paramType));
//     } while (token.type == TokenType::COMMA && (advance(), true));
//   }

//   consume(TokenType::RIGHT_PAREN);

//   std::unique_ptr<Type> funcType = std::make_unique<FunctionType>(
//       std::move(paramTypes), std::move(returnType));

//   // Prototype
//   if (token.type == TokenType::SEMICOLON) {
//     consume(TokenType::SEMICOLON);
//     return new FuncDecl(funcName, std::move(paramNames), nullptr,
//                         std::move(funcType));
//   }

//   // Function definition
//   consume(TokenType::LEFT_BRACE);
//   std::unique_ptr<Block> body = std::make_unique<Block>();
//   BlockItem *nextItem;

//   while (token.type != TokenType::RIGHT_BRACE) {
//     nextItem = parseBlockItem();
//     if (!nextItem)
//       break;
//     body->addItem(std::unique_ptr<BlockItem>(nextItem));
//   }

//   consume(TokenType::RIGHT_BRACE);

//   return new FuncDecl(funcName, std::move(paramNames), std::move(body),
//                       std::move(funcType));
// }

FuncDecl *Parser::parseFuncDeclOrProto(std::string funcName,
                                       std::unique_ptr<Type> returnType,
                                       std::vector<std::string> paramNames) {
  // Prototype
  if (token.type == TokenType::SEMICOLON) {
    consume(TokenType::SEMICOLON);
    return new FuncDecl(funcName, std::move(paramNames), nullptr,
                        std::move(returnType));
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
                      std::move(returnType));
}


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
        token.type == TokenType::SIGNED || token.type == TokenType::UNSIGNED ||
        token.type == TokenType::LONG || token.type == TokenType::DOUBLE) {
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
      token.type == TokenType::INCREMENT ||
      token.type == TokenType::DECREMENT || token.type == TokenType::MUL ||
      token.type == TokenType::BITWISE_AND) {
    TokenType op = token.type;
    int prec = getPrecedence(op);
    advance();
    Expr *right = parseExpr(prec + 1);

    if (op == TokenType::MUL) {
      left = new Dereference(std::unique_ptr<Expr>(right));
    } else if (op == TokenType::BITWISE_AND) {
      left = new AddrOf(std::unique_ptr<Expr>(right));
    } else {
      left = new UnaryOp(op, std::unique_ptr<Expr>(right), false);
    }

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

  if (token.type == TokenType::NUM || token.type == TokenType::LONG_CONST ||
      token.type == TokenType::FLOAT_CONST ||
      token.type == TokenType::UNSIGNED_LONG_CONST ||
      token.type == TokenType::UNSIGNED_INT_CONST) {
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

std::tuple<std::unique_ptr<Type>, StorageClass, TypeQualifier>
Parser::parseTypeAndStorageClass() {
  std::vector<TokenType> specifiers;

  // Collect valid specifiers
  while (token.type == TokenType::STATIC || token.type == TokenType::EXTERN ||
         token.type == TokenType::CONST || token.type == TokenType::INT ||
         token.type == TokenType::SIGNED || token.type == TokenType::UNSIGNED ||
         token.type == TokenType::LONG || token.type == TokenType::DOUBLE) {
    specifiers.push_back(token.type);
    advance();
  }

  // Flags
  bool seenConst = false;
  bool seenInt = false;
  bool seenDouble = false;
  bool seenLong = false;
  bool seenSigned = false;
  bool seenUnsigned = false;
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
    case TokenType::SIGNED:
      if (seenSigned || seenUnsigned) {
        std::cerr << "ERROR: Cannot combine 'signed' and 'unsigned'\n";
        exit(1);
      }
      seenSigned = true;
      break;
    case TokenType::UNSIGNED:
      if (seenSigned || seenUnsigned) {
        std::cerr << "ERROR: Cannot combine 'signed' and 'unsigned'\n";
        exit(1);
      }
      seenUnsigned = true;
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
    case TokenType::DOUBLE:
      if (seenDouble) {
        std::cerr << "ERROR: Duplicate 'double' specifier\n";
        exit(1);
      }
      seenDouble = true;
      break;
    default:
      error();
    }
  }

  // Handle 'double'
  if (seenDouble) {
    if (seenInt || seenLong || seenUnsigned || seenSigned) {
      std::cerr << "ERROR: Invalid combination with 'double'\n";
      exit(1);
    }
    // return {std::make_unique<DoubleType>(), storage,
    //         seenConst ? TypeQualifier::CONST : TypeQualifier::NONE};

    // Handle pointer types (e.g., int*, long**, etc.)
    std::unique_ptr<Type> baseType = std::make_unique<DoubleType>();
    while (token.type == TokenType::MUL) {
      consume(TokenType::MUL);
      baseType = std::make_unique<PointerType>(std::move(baseType));
    }

    return {std::move(baseType), storage,
            seenConst ? TypeQualifier::CONST : TypeQualifier::NONE};
  }

  // Determine base type for int/long/signed/unsigned
  std::unique_ptr<Type> baseType;

  if (seenUnsigned) {
    if (seenLong)
      baseType = std::make_unique<UnsignedLongType>();
    else
      baseType = std::make_unique<UnsignedIntType>();
  } else if (seenSigned) {
    if (seenLong)
      baseType = std::make_unique<LongType>();
    else
      baseType = std::make_unique<IntType>();
  } else {
    if (seenLong)
      baseType = std::make_unique<LongType>();
    else
      baseType = std::make_unique<IntType>();
  }

  // Handle pointer types (e.g., int*, long**, etc.)
  while (token.type == TokenType::MUL) {
    consume(TokenType::MUL);
    baseType = std::make_unique<PointerType>(std::move(baseType));
  }

  return {std::move(baseType), storage,
          seenConst ? TypeQualifier::CONST : TypeQualifier::NONE};
}

Expr *Parser::parseConstant() {
  std::string tokentext = token.value.value();

  // Handle unsigned int
  if (token.type == TokenType::UNSIGNED_INT_CONST) {
    uint64_t v;
    try {
      v = std::stoull(tokentext);
    } catch (const std::exception &) {
      std::cerr << "ERROR: Invalid unsigned int constant value\n";
      exit(1);
    }

    if (v > UINT32_MAX) {
      std::cerr << "ERROR: Value too large for unsigned int\n";
      exit(1);
    }

    consume(TokenType::UNSIGNED_INT_CONST);
    return new ConstUnsignedInt(static_cast<unsigned int>(v));
  }

  // Handle unsigned long
  if (token.type == TokenType::UNSIGNED_LONG_CONST) {
    uint64_t v;
    try {
      v = std::stoull(tokentext);
    } catch (const std::exception &) {
      std::cerr << "ERROR: Invalid unsigned long constant value\n";
      exit(1);
    }

    consume(TokenType::UNSIGNED_LONG_CONST);
    return new ConstUnsignedLong(static_cast<unsigned long>(v));
  }

  // Handle signed long
  if (token.type == TokenType::LONG_CONST) {
    long long v;
    try {
      v = std::stoll(tokentext);
    } catch (const std::exception &) {
      std::cerr << "ERROR: Invalid long constant value\n";
      exit(1);
    }

    consume(TokenType::LONG_CONST);
    return new ConstLong(static_cast<long>(v));
  }

  // Handle floating-point (double) constant
  if (token.type == TokenType::FLOAT_CONST) {
    double v;
    try {
      v = std::stod(tokentext);
    } catch (const std::exception &) {
      std::cerr << "ERROR: Invalid floating-point constant value\n";
      exit(1);
    }

    consume(TokenType::FLOAT_CONST);
    return new ConstDouble(v);
  }

  // Default: plain NUM (could be int or long)
  if (token.type == TokenType::NUM) {
    uint64_t v;
    try {
      v = std::stoull(tokentext);
    } catch (const std::exception &) {
      std::cerr << "ERROR: Invalid numeric constant value\n";
      exit(1);
    }

    if (v <= INT32_MAX) {
      consume(TokenType::NUM);
      return new ConstInt(static_cast<int>(v));
    } else if (v <= INT64_MAX) {
      consume(TokenType::NUM);
      return new ConstLong(static_cast<long>(v));
    } else {
      std::cerr << "ERROR: Constant too large for signed long\n";
      exit(1);
    }
  }

  error(); // fallback
  return nullptr;
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