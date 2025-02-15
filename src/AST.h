#pragma once

#include "lexer.h"
#include "TAC.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class AST;
class Expr;
class Factor;
class BinaryOp;
class WithDecl;

using namespace std;

enum class StmtType { ASSIGN, EXPR, RETURN };

class ASTVisitor {
public:
  virtual void visit(AST &) {};
  virtual void visit(Expr &) {};
  virtual void visit(Factor &) = 0;
  virtual void visit(BinaryOp &) = 0;
  virtual void visit(WithDecl &) = 0;
};

//////////////////////////////////////////////////////////////////////////

// Defining AST
class AST {
public:
  static int tempVarCounter; // Counter for temporary variables
  virtual ~AST() = default;
  virtual std::vector<TAC> generateTAC(std::string &tempVar) = 0;
};

//////////////////////////////////////////////////////////////////////////

// Defining Expr
class Expr : public AST {
public:
  virtual ~Expr() = default;
  virtual void print() = 0;
};

//////////////////////////////////////////////////////////////////////////

// Integer literal
class IntLiteral : public Expr {
public:
  int value;
  IntLiteral(int val) : value(val) {}
  void print() { cout << "IntLiteral: " << value << endl; }
  
  std::vector<TAC> generateTAC(std::string &tempVar) override {
      std::vector<TAC> code;
      tempVar = "t" + std::to_string(tempVarCounter++);
      code.push_back(TAC("li", std::to_string(value), "", tempVar));
      return code;
  }
};

//////////////////////////////////////////////////////////////////////////

// Variable reference
class Variable : public Expr {
public:
  std::string name;
  Variable(const std::string &name) : name(name) {}
  void print() { cout << "Variable: " << name << endl; }
  
  std::vector<TAC> generateTAC(std::string &tempVar) override {
      std::vector<TAC> code;
      tempVar = "t" + std::to_string(tempVarCounter++);
      code.push_back(TAC("load", name, "", tempVar));
      return code;
    }
};

//////////////////////////////////////////////////////////////////////////

// Binary operation (e.g., `a + b`)
class BinaryOp : public Expr {
public:
  char op; // Operator like '+', '-', '*', '/'
  std::unique_ptr<Expr> left, right;

  BinaryOp(char op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : op(op), left(std::move(left)), right(std::move(right)) {}

    void print() {
      cout << "BinaryOp: ";
      left->print();
      cout << " " << op << " ";
      right->print();
      cout << endl;
    }

 std::vector<TAC> generateTAC(std::string &tempVar) override {
        std::vector<TAC> code;
        std::string leftTemp, rightTemp;
        
        // Generate TAC for left operand
        auto leftCode = left->generateTAC(leftTemp);
        code.insert(code.end(), leftCode.begin(), leftCode.end());

        // Generate TAC for right operand
        auto rightCode = right->generateTAC(rightTemp);
        code.insert(code.end(), rightCode.begin(), rightCode.end());

        // Create a new temporary variable
        tempVar = "t" + std::to_string(tempVarCounter++);
        
        // Map TokenType to TAC operation
        std::string opStr;
        #define TOKEN_TO_STRING(token, str) \
        case TokenType::token:          \
          opStr = str;               \
            break;

        switch(op){
          TOKEN_TO_STRING(PLUS, "+")
          TOKEN_TO_STRING(MINUS, "-")
          TOKEN_TO_STRING(MUL, "*")
          TOKEN_TO_STRING(DIV, "/")
          TOKEN_TO_STRING(MOD, "%")
          TOKEN_TO_STRING(BITWISE_AND, "&")
          TOKEN_TO_STRING(BITWISE_OR, "|")
          TOKEN_TO_STRING(BITWISE_XOR, "^")
          TOKEN_TO_STRING(LEFT_SHIFT, "<<")
          TOKEN_TO_STRING(RIGHT_SHIFT, ">>")
          TOKEN_TO_STRING(LOGICAL_AND, "&&")
          TOKEN_TO_STRING(LOGICAL_OR, "||")
          TOKEN_TO_STRING(EQUAL_EQUAL, "==")
          TOKEN_TO_STRING(NOT_EQUAL, "!=")
          TOKEN_TO_STRING(LESS_THAN, "<")
          TOKEN_TO_STRING(GREATER_THAN, ">")
          TOKEN_TO_STRING(LESS_THAN_EQUAL, "<=")
          TOKEN_TO_STRING(GREATER_THAN_EQUAL, ">=")
        }
        #undef TOKEN_TO_STRING

        // Emit TAC for binary operation
        code.push_back(TAC(opStr, leftTemp, rightTemp, tempVar));

        return code;
    }
};

//////////////////////////////////////////////////////////////////////////

class UnaryOp : public Expr {
public:
  TokenType op;
  std::unique_ptr<Expr> expr;
  UnaryOp(TokenType op, std::unique_ptr<Expr> expr)
      : op(op), expr(std::move(expr)) {}
    void print(){
        cout << "UnaryOp: " << TokenStr[(int)op] << " ";
        expr->print();
    }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    std::string exprTemp;

    // Generate TAC for the operand
    auto exprCode = expr->generateTAC(exprTemp);
    code.insert(code.end(), exprCode.begin(), exprCode.end());

    // Create a new temporary variable
    tempVar = "t" + std::to_string(tempVarCounter++);

    // Map TokenType to TAC operation
    std::string opStr;
    if (op == TokenType::MINUS) opStr = "NEG";
    else if (op == TokenType::COMPLEMENT) opStr = "~";

    // Emit TAC for unary operation
    code.push_back(TAC(opStr, exprTemp, "", tempVar));

    return code;
  }
};
// Initialize static member

//////////////////////////////////////////////////////////////////////////

// Defining Statement
class Stmt : public AST {
public:
  virtual ~Stmt() = default;
  virtual void print() = 0;
  virtual StmtType getType() const = 0;
  virtual std::vector<TAC> generateTAC(std::string &tempVar) = 0;
};

//////////////////////////////////////////////////////////////////////////

// Variable assignment (e.g., `x = 5;`)
class AssignStmt : public Stmt {
public:
  std::string name;
  std::unique_ptr<Expr> value;

  AssignStmt(std::string name, std::unique_ptr<Expr> value)
      : name(std::move(name)), value(std::move(value)) {}

  void print() {
    cout << "AssignStmt: " << name << " = ";
    value->print();
  }
  StmtType getType() const override { return StmtType::ASSIGN; }
};

//////////////////////////////////////////////////////////////////////////

// Expression statement (e.g., `foo(42);`)
class ExprStmt : public Stmt {
public:
  std::unique_ptr<Expr> expr;

  ExprStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}

  void print() {
    cout << "ExprStmt: ";
    expr->print();
  }
  StmtType getType() const override { return StmtType::EXPR; }
};

//////////////////////////////////////////////////////////////////////////

class ReturnStmt : public Stmt {
public:
  std::unique_ptr<Expr> expr;

  ReturnStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}

  void print() {
    cout << "ReturnStmt: ";
    expr->print();
  }
  StmtType getType() const override { return StmtType::RETURN; }

  std::vector<TAC> generateTAC(string &tempvar) override{
      std::vector<TAC> code;
      std::string tempVar;
      
      // Generate TAC for the return expression
      auto exprCode = expr->generateTAC(tempVar);
      code.insert(code.end(), exprCode.begin(), exprCode.end());

      // Emit TAC for return statement
      code.push_back(TAC("RETURN", tempVar, "", ""));
      return code;
  }
};

//////////////////////////////////////////////////////////////////////////

class Func : public AST {
public:
  std::string name;
  std::vector<std::unique_ptr<Stmt>> body;
  // std::unique_ptr<Stmt> body;

  // Func(std::string name, std::unique_ptr<Stmt> body)
  // : name(std::move(name)), body(std::move(body)) {}

  Func(std::string name, std::vector<std::unique_ptr<Stmt>> body)
      : name(std::move(name)), body(std::move(body)) {}

 std::vector<TAC> generateTAC(std::string &tempVar) override{
      std::vector<TAC> code;
      // std::cout << "Function " << name << ":\n";
      for (auto &stmt : body) {
          std::string tempVar;
          auto stmtCode = stmt->generateTAC(tempVar);
          code.insert(code.end(), stmtCode.begin(), stmtCode.end());
      }
      return code;
  }
};

class ASTProgram : public AST {
public:
  std::vector<std::unique_ptr<Func>> functions;

  void addFunction(std::unique_ptr<Func> func) {
    functions.push_back(std::move(func));
  }

  void print() {
    for (auto &func : functions) {
      cout << "Function: " << func->name << endl;
      // for(auto &stmt : func->body){
      //     cout << "Statement: " << stmt->name << endl;
      // }
      // func.body->print();
      cout << "Body: ";
      for (auto &stmt : func->body) {
        stmt->print();
      }
    }
  }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    for (auto &func : functions) {
      std::string tempVar;
      auto funcCode = func->generateTAC(tempVar);
      code.insert(code.end(), funcCode.begin(), funcCode.end());
    }
    return code;
  }
};