#pragma once

#include "lexer.h"
#include "TAC.h"
#include "SymbolTable.h"
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

enum class StmtType { EXPR, RETURN, NULL_STMT };

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
  virtual void resolveSymbol(SymbolTable &symTab) {}
};

//////////////////////////////////////////////////////////////////////////

// Defining Expr
class Expr : public AST {
public:
  virtual ~Expr() = default;
  virtual void print() = 0;
};

//////////////////////////////////////////////////////////////////////////

// Defining Block Items (eg Statements, Declarations)
class BlockItem : public AST {
  public:
    virtual ~BlockItem() = default;
    virtual void print() = 0;
    virtual std::vector<TAC> generateTAC(std::string &tempVar) = 0;
};
  


class Block : public AST {
  public:
  std::vector<std::unique_ptr<BlockItem>> items;

  void addItem(std::unique_ptr<BlockItem> item) {
    items.push_back(std::move(item));
  }

  void print() {
    for (auto &item : items) {
      item->print();
    }
  }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    for (auto &item : items) {
      std::string tempVar;
      auto itemCode = item->generateTAC(tempVar);
      code.insert(code.end(), itemCode.begin(), itemCode.end());
    }
    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    for (auto &item : items) {
      item->resolveSymbol(symTab);
    }
  }

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

  void resolveSymbol(SymbolTable &symTab) override {}
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

  void resolveSymbol(SymbolTable &symTab) override {
    if (!symTab.resolve(name)) {
      std::cerr << "ERROR: Undeclared variable '" << name << "'" << std::endl;
      exit(1);
    }
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

        if (op == TokenType::LOGICAL_AND || op == TokenType::LOGICAL_OR) {
          // Create labels
          std::string falseLabel = "L" + std::to_string(AST::tempVarCounter++);
          std::string trueLabel = "L" + std::to_string(AST::tempVarCounter++);
          std::string endLabel = "L" + std::to_string(AST::tempVarCounter++);
          
          // Create result temporary variable
          tempVar = "t" + std::to_string(AST::tempVarCounter++);
  
          if (op == TokenType::LOGICAL_AND) {
              // if left is false, jump to falseLabel
              code.push_back(TAC("beq", leftTemp, "0", falseLabel));
          } else { // LOGICAL_OR
              // if left is true, jump to trueLabel
              code.push_back(TAC("bne", leftTemp, "0", trueLabel));
          }
  
          // Generate TAC for right operand
          auto rightCode = right->generateTAC(rightTemp);
          code.insert(code.end(), rightCode.begin(), rightCode.end());
  
          // Assign result of right operand to tempVar
          code.push_back(TAC("move", rightTemp, "", tempVar));
          code.push_back(TAC("jmp", "", "", endLabel));
  
          // False label: result is 0
          code.push_back(TAC("label", falseLabel, "", ""));
          code.push_back(TAC("li", "0", "", tempVar));
          code.push_back(TAC("jmp", "", "", endLabel));
  
          // True label: result is 1
          code.push_back(TAC("label", trueLabel, "", ""));
          code.push_back(TAC("li", "1", "", tempVar));
  
          // End label
          code.push_back(TAC("label", endLabel, "", ""));
  
          return code;
      }

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

    void resolveSymbol(SymbolTable &symTab) override {
      left->resolveSymbol(symTab);
      right->resolveSymbol(symTab);
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
    else if (op == TokenType::LOGICAL_NOT) {
      opStr = "seq";  // Set equal to zero
      code.push_back(TAC(opStr, exprTemp, "0", tempVar));
      return code;
  }
    // Emit TAC for unary operation
    code.push_back(TAC(opStr, exprTemp, "", tempVar));

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    expr->resolveSymbol(symTab);
  }
};
// Initialize static member

//////////////////////////////////////////////////////////////////////////

// Variable assignment (e.g., `x = 5;`)
class Assignment : public Expr {
  public:
  std::unique_ptr<Expr> name;
  std::unique_ptr<Expr> value;
  
  Assignment(std::unique_ptr<Expr> name, std::unique_ptr<Expr> value)
  : name(std::move(name)), value(std::move(value)) {}
  
  void print() {
    cout << "AssignStmt: "; 
    name->print();
    cout << " = ";
    value->print();
  }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    std::string nameTemp, valueTemp;
    
    // Generate TAC for the name
    auto nameCode = name->generateTAC(nameTemp);
    code.insert(code.end(), nameCode.begin(), nameCode.end());
    
    // Generate TAC for the value
    auto valueCode = value->generateTAC(valueTemp);
    code.insert(code.end(), valueCode.begin(), valueCode.end());
    
    // Emit TAC for assignment
    code.push_back(TAC("store", valueTemp, "", nameTemp));
    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    name->resolveSymbol(symTab);
    value->resolveSymbol(symTab);
  }
};


//////////////////////////////////////////////////////////////////////////

class Decl : public BlockItem {
  public:
    std::string name;
    std::unique_ptr<Expr> initializer;  // Optional initializer
  
    Decl(std::string name, std::unique_ptr<Expr> initializer = nullptr)
        : name(std::move(name)), initializer(std::move(initializer)) {}
  
    void print() override {
      cout << "Declaration: " << name;
      if (initializer) {
        cout << " = ";
        initializer->print();
      }
      cout << endl;
    }
  
    std::vector<TAC> generateTAC(std::string &tempVar) override {
      std::vector<TAC> code;
      tempVar = name;  // Variable name acts as the destination
  
      if (initializer) {
        std::string initTemp;
        auto initCode = initializer->generateTAC(initTemp);
        code.insert(code.end(), initCode.begin(), initCode.end());
        code.push_back(TAC("store", initTemp, "", tempVar));
      }
      return code;
    }

    void resolveSymbol(SymbolTable &symTab) override {
      if (!symTab.declare(name)) {
        std::cerr << "ERROR: Redeclaration of variable '" << name << "'" << std::endl;
        exit(1);
      }
      if (initializer) {
        initializer->resolveSymbol(symTab);
      }
    }
};  

//////////////////////////////////////////////////////////////////////////

// Defining Statement
class Stmt : public BlockItem {
public:
  virtual ~Stmt() = default;
  virtual void print() = 0;
  virtual StmtType getType() const = 0;
  virtual std::vector<TAC> generateTAC(std::string &tempVar) = 0;
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

  std::vector<TAC> generateTAC(std::string &tempvar) override{
      std::vector<TAC> code;
      std::string tempVar;
      
      // Generate TAC for the expression
      auto exprCode = expr->generateTAC(tempVar);
      code.insert(code.end(), exprCode.begin(), exprCode.end());

      code.push_back(TAC("EXPR", tempVar, "", ""));
      return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    expr->resolveSymbol(symTab);
  }
  
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

  void resolveSymbol(SymbolTable &symTab) override {
    expr->resolveSymbol(symTab);
  }

};

class NullStmt : public Stmt {
public:
  void print() { cout << "NullStmt" << endl; }
  StmtType getType() const override { return StmtType::NULL_STMT; }

  void resolveSymbol(SymbolTable &symTab) override {}
};

//////////////////////////////////////////////////////////////////////////

// class Func : public AST {
// public:
//   std::string name;
//   std::vector<std::unique_ptr<Block>> body;
//   // std::unique_ptr<Stmt> body;

//   // Func(std::string name, std::unique_ptr<Stmt> body)
//   // : name(std::move(name)), body(std::move(body)) {}

//   Func(std::string name, std::vector<std::unique_ptr<Block>> body)
//       : name(std::move(name)), body(std::move(body)) {}
  
//   void print() {
//     cout << "Function: " << name << endl;
//     cout << "Body: " << endl;
//     body->print();
//   }

//  std::vector<TAC> generateTAC(std::string &tempVar) override{
//       std::vector<TAC> code;
//       // std::cout << "Function " << name << ":\n";
//       for (auto &stmt : body) {
//           std::string tempVar;
//           auto stmtCode = stmt->generateTAC(tempVar);
//           code.insert(code.end(), stmtCode.begin(), stmtCode.end());
//       }
//       return code;
//   }
// };

class Func : public AST {
  public:
    std::string name;
    std::unique_ptr<Block> body;
  
    Func(std::string name, std::unique_ptr<Block> body)
        : name(std::move(name)), body(std::move(body)) {}
  
    void print() {
      cout << "Function: " << name << endl;
      cout << "Body: " << endl;
      body->print();
    }
  
    std::vector<TAC> generateTAC(std::string &tempVar) override {
      std::vector<TAC> code;
      auto bodyCode = body->generateTAC(tempVar);
      code.insert(code.end(), bodyCode.begin(), bodyCode.end());
      return code;
    }

    void resolveSymbol(SymbolTable &symTab) override {
      symTab.enterScope();
      body->resolveSymbol(symTab);
      symTab.exitScope();
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
      func->print();
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

  void resolveSymbol(SymbolTable &symTab) override {
    for (auto &func : functions) {
      func->resolveSymbol(symTab);
    }
  }
};