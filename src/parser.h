#pragma once

#include "AST.h"
#include "Type.h"
#include "lexer.h"
#include <iostream>

using namespace std;

struct Declarator {
  enum Kind { IDENT, POINTER, FUNCTION } kind;
  std::string ident;                 // Only if kind == IDENT
  std::unique_ptr<Declarator> inner; // For POINTER or FUNCTION
  std::vector<std::pair<std::unique_ptr<Type>, std::string>>
      params; // For FUNCTION
};

class Parser {
private:
  Lexer &lexer;
  Token token;
  bool HasError;
  void advance();
  void error();
  bool expect(TokenType type);
  bool consume(TokenType type);
  int getPrecedence(TokenType op);
  bool isBinaryOp(TokenType type);
  bool isCompoundAssignOp(TokenType type);
  bool isTypeSpecifier(TokenType type);

  ASTProgram *parseProgram();
  FuncDecl *parseFunction();
  Stmt *parseStatement();
  BlockItem *parseBlockItem();
  Block *parseBlock();
  Expr *parseExpr(int minPrec = 0);
  Expr *parseTerm();
  Expr *parseFactor();
  Expr *parseCast();
  Expr *parseConstant();
  Declaration *parseDeclaration();
  std::unique_ptr<Declarator> parseDeclarator();
  std::unique_ptr<Declarator>
  parseFunctionDeclarator(std::unique_ptr<Declarator> inner);
  std::tuple<std::string, std::unique_ptr<Type>, std::vector<std::string>>
  processDeclarator(std::unique_ptr<Declarator> d, std::unique_ptr<Type> base);
  VarDecl *parseVarDecl(const string &varName, std::unique_ptr<Type> type);
  FuncDecl *parseFuncDecl(const string &funcName,
                          std::unique_ptr<Type> returnType);
  // FuncDecl *parseFuncDeclOrProto(const string &funcName,
  // std::unique_ptr<Type> returnType);
  FuncDecl *parseFuncDeclOrProto(std::string name, std::unique_ptr<Type> type,
                                 std::vector<std::string> paramNames);
  ExprStmt *parseExprStmt();
  TypeSpecifier parseType(std::vector<TypeSpecifier> &types);
  std::tuple<std::unique_ptr<Type>, StorageClass, TypeQualifier>
  parseTypeAndStorageClass();

public:
  Parser(Lexer &lex);
  ASTProgram *parse();
};