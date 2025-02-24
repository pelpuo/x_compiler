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

enum class StmtType { EXPR, RETURN, NULL_STMT, IF_STMT, BLOCK, WHILE, FOR, DO_WHILE, BREAK, CONTINUE, SWITCH, CASE, DEFAULT, DECL, FUNC_DECL, WITH_DECL };

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
  static std::vector<std::pair<std::string, std::string>> loopLabels; // Loop labels for break and continue
  static std::vector<std::string> switchLabels; // Switch labels for break
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
  
class Declaration : public BlockItem {
  public:
    virtual ~Declaration() = default;
    virtual void print() = 0;
    virtual std::vector<TAC> generateTAC(std::string &tempVar) = 0;
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

class ArgList {
  public:
    std::vector<std::unique_ptr<Expr>> args;

    void addArg(std::unique_ptr<Expr> arg) {
      args.push_back(std::move(arg));
    }
  
    void print() {
      for (size_t i = 0; i < args.size(); ++i) {
        if (args[i]) args[i]->print();
        if (i < args.size() - 1) cout << ", ";
      }
    }
  
    std::vector<TAC> generateTAC(std::vector<TAC> &code) {
      for (auto &arg : args) {
        std::string tempVar;
        auto argCode = arg->generateTAC(tempVar);
        code.insert(code.end(), argCode.begin(), argCode.end());
  
        // Push argument before the function call
        code.push_back(TAC("arg", tempVar, "", ""));
      }
      return code;
    }
  
    void resolveSymbol(SymbolTable &symTab) {
      for (auto &arg : args) {
        arg->resolveSymbol(symTab);
      }
    }
  };
  
//////////////////////////////////////////////////////////////////////////

// Variable reference (e.g., `x`)
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

    tempVar = valueTemp;
    
    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    name->resolveSymbol(symTab);
    value->resolveSymbol(symTab);
  }
};

//////////////////////////////////////////////////////////////////////////

// Compound assignment (e.g., `x += 5;`)
class CompoundAssignment : public Expr {
  public:
    std::unique_ptr<Expr> left;
    TokenType op;
    std::unique_ptr<Expr> right;
  
    CompoundAssignment(TokenType op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}
  
    void print() {
      cout << "CompoundAssignStmt: ";
      left->print();
      cout << " " << TokenStr[(int)op] << " ";
      right->print();
    }
  
    std::vector<TAC> generateTAC(std::string &tempVar) override {
      std::vector<TAC> code;
      std::string nameTemp, valueTemp, resultTemp;
  
      // Generate TAC for the name
      auto nameCode = left->generateTAC(nameTemp);
      code.insert(code.end(), nameCode.begin(), nameCode.end());
  
      // Generate TAC for the value
      auto valueCode = right->generateTAC(valueTemp);
      code.insert(code.end(), valueCode.begin(), valueCode.end());
  
      // Create a new temporary variable for the result
      resultTemp = "t" + std::to_string(tempVarCounter++);
  
      // Map TokenType to TAC operation
      std::string opStr;
      #define TOKEN_TO_STRING(token, str) \
      case TokenType::token:          \
        opStr = str;               \
          break;
  
      switch(op){
        TOKEN_TO_STRING(PLUS_EQUAL, "+")
        TOKEN_TO_STRING(MINUS_EQUAL, "-")
        TOKEN_TO_STRING(MUL_EQUAL, "*")
        TOKEN_TO_STRING(DIV_EQUAL, "/")
        TOKEN_TO_STRING(MOD_EQUAL, "%")
        TOKEN_TO_STRING(AND_EQUAL, "&")
        TOKEN_TO_STRING(OR_EQUAL, "|")
        TOKEN_TO_STRING(XOR_EQUAL, "^")
        TOKEN_TO_STRING(LEFT_SHIFT_EQUAL, "<<")
        TOKEN_TO_STRING(RIGHT_SHIFT_EQUAL, ">>")
        default:
          std::cerr << "ERROR: Invalid compound assignment operator" << std::endl;
          exit(1);
      }
      #undef TOKEN_TO_STRING
  
      // Emit TAC for compound assignment operation
      code.push_back(TAC(opStr, nameTemp, valueTemp, resultTemp));
      code.push_back(TAC("store", resultTemp, "", nameTemp));
  
      return code;
    }
  
    void resolveSymbol(SymbolTable &symTab) override {
      left->resolveSymbol(symTab);
      right->resolveSymbol(symTab);
    }
  };

//////////////////////////////////////////////////////////////////////////

class TernaryOp : public Expr {
  public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> trueExpr;
    std::unique_ptr<Expr> falseExpr;
  
    TernaryOp(std::unique_ptr<Expr> condition, std::unique_ptr<Expr> trueExpr, std::unique_ptr<Expr> falseExpr)
        : condition(std::move(condition)), trueExpr(std::move(trueExpr)), falseExpr(std::move(falseExpr)) {}
  
    void print() {
      cout << "TernaryOp: ";
      condition->print();
      cout << " ? ";
      trueExpr->print();
      cout << " : ";
      falseExpr->print();
    }
  
    std::vector<TAC> generateTAC(std::string &tempVar) override {
      std::vector<TAC> code;
      std::string condTemp;
  
      // 1. Generate TAC for the condition
      auto condCode = condition->generateTAC(condTemp);
      code.insert(code.end(), condCode.begin(), condCode.end());
  
      // 2. Create labels
      std::string trueLabel = "L" + std::to_string(AST::tempVarCounter++);
      std::string falseLabel = "L" + std::to_string(AST::tempVarCounter++);
      std::string endLabel = "L" + std::to_string(AST::tempVarCounter++);
  
      // 3. Conditional jump: Jump to falseLabel if condition is false
      code.push_back(TAC("beqz", condTemp, falseLabel, ""));
  
      // 4. True expression
      code.push_back(TAC("label", trueLabel, "", ""));
      std::string trueTemp;
      auto trueCode = trueExpr->generateTAC(trueTemp);
      code.insert(code.end(), trueCode.begin(), trueCode.end());
      code.push_back(TAC("move", trueTemp, "", tempVar)); // Store result in tempVar
      code.push_back(TAC("jmp", "", "", endLabel)); // Jump to end
  
      // 5. False expression
      code.push_back(TAC("label", falseLabel, "", ""));
      std::string falseTemp;
      auto falseCode = falseExpr->generateTAC(falseTemp);
      code.insert(code.end(), falseCode.begin(), falseCode.end());
      code.push_back(TAC("move", falseTemp, "", tempVar)); // Store result in tempVar
  
      // 6. End label
      code.push_back(TAC("label", endLabel, "", ""));
  
      return code;
    }
  
  
    void resolveSymbol(SymbolTable &symTab) override {
      condition->resolveSymbol(symTab);
      trueExpr->resolveSymbol(symTab);
      falseExpr->resolveSymbol(symTab);
    }
  };

//////////////////////////////////////////////////////////////////////////

class FuncCall : public Expr {
public:
  std::string name;
  std::unique_ptr<ArgList> args;

  FuncCall(const std::string &name, std::unique_ptr<ArgList> args)
      : name(name), args(std::move(args)) {}

  void print() {
    cout << "FuncCall: " << name << "(";
    if (args) {
      args->print();
    }
    cout << ")" << endl;
  }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    // std::string argsTemp;

    // Generate TAC for the argument list
    if (args) {
      args->generateTAC(code);
      // auto argsCode = args->generateTAC(code);
      // code.insert(code.end(), argsCode.begin(), argsCode.end());
    }

    // Create a new temporary variable for the result
    tempVar = "t" + std::to_string(tempVarCounter++);

    // Emit TAC for function call
    code.push_back(TAC("call", name, "", tempVar));

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    if (!symTab.isFunction(name)) {
      std::cerr << "ERROR: Undeclared function '" << name << "'" << std::endl;
      exit(1);
    }
    const auto &paramTypes = symTab.getFunctionParams(name);
    if (args && args->args.size() != paramTypes->size()) {
      std::cerr << "ERROR: Argument count mismatch for function '" << name << "'" << std::endl;
      exit(1);
    }
    if (args) {
      args->resolveSymbol(symTab);
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

class Block : public Stmt {
  public:
  std::vector<std::unique_ptr<BlockItem>> items;

  StmtType getType() const override { return StmtType::BLOCK; }

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
    symTab.enterScope();
    for (auto &item : items) {
      item->resolveSymbol(symTab);
    }
    symTab.exitScope();
  }

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

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    return {};
  }
};

//////////////////////////////////////////////////////////////////////////

class IfStmt : public Stmt {
public:
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> thenBlock;
  std::unique_ptr<Stmt> elseBlock;

  IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBlock,
         std::unique_ptr<Stmt> elseBlock)
      : condition(std::move(condition)), thenBlock(std::move(thenBlock)),
        elseBlock(std::move(elseBlock)) {}

  void print() {
    cout << "IfStmt: ";
    condition->print();
    cout << "Then: ";
    thenBlock->print();
    if (elseBlock) {
      cout << "Else: ";
      elseBlock->print();
    }
  }

  StmtType getType() const override { return StmtType::IF_STMT; }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    std::string condTemp;

    // 1. Generate TAC for the condition, storing the result in condTemp
    auto condCode = condition->generateTAC(condTemp);
    code.insert(code.end(), condCode.begin(), condCode.end());

    // 2. Create labels
    std::string thenLabel = "L" + std::to_string(AST::tempVarCounter++); // Label for the 'then' block
    std::string elseLabel = ""; // Initialize to empty string
    std::string endLabel = "L" + std::to_string(AST::tempVarCounter++);

    if (elseBlock) {
      elseLabel = "L" + std::to_string(AST::tempVarCounter++);
    }

    // 3. Conditional jump: Jump to elseLabel if condition is false (0)
    code.push_back(TAC("beqz", condTemp, elseLabel.empty() ? endLabel : elseLabel, "")); // Jump to end if no else


    // 4. Then block
    code.push_back(TAC("label", thenLabel, "", "")); // Label the then block
    auto thenCode = thenBlock->generateTAC(tempVar);
    code.insert(code.end(), thenCode.begin(), thenCode.end());

    // 5. Jump to end if there's an else block
    if (elseBlock) {
      code.push_back(TAC("jmp", "", "", endLabel));
    }

    // 6. Else block (if it exists)
    if (elseBlock) {
      code.push_back(TAC("label", elseLabel, "", "")); // Label the else block
      auto elseCode = elseBlock->generateTAC(tempVar);
      code.insert(code.end(), elseCode.begin(), elseCode.end());
    }

    // 7. End label
    code.push_back(TAC("label", endLabel, "", ""));

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    condition->resolveSymbol(symTab);
    symTab.enterScope();
    thenBlock->resolveSymbol(symTab);
    if (elseBlock) {
      elseBlock->resolveSymbol(symTab);
    }
    symTab.exitScope();
  }
};

//////////////////////////////////////////////////////////////////////////

class WhileStmt : public Stmt {
public:
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> body;

  WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
      : condition(std::move(condition)), body(std::move(body)) {}

  void print() {
    cout << "WhileStmt: ";
    condition->print();
    cout << "Body: ";
    body->print();
  }

  StmtType getType() const override { return StmtType::WHILE; }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    std::string condTemp;

    // 1. Create labels
    std::string startLabel = "L" + std::to_string(AST::tempVarCounter++);
    std::string endLabel = "L" + std::to_string(AST::tempVarCounter++);

  // Push loop labels for break/continue support
    AST::loopLabels.push_back({startLabel, endLabel});

    // 2. Start label
    code.push_back(TAC("label", startLabel, "", ""));

    // 3. Generate TAC for the condition, storing the result in condTemp
    auto condCode = condition->generateTAC(condTemp);
    code.insert(code.end(), condCode.begin(), condCode.end());

    // 4. Conditional jump: Jump to endLabel if condition is false (0)
    code.push_back(TAC("beqz", condTemp, endLabel, ""));

    // 5. Body
    auto bodyCode = body->generateTAC(tempVar);
    code.insert(code.end(), bodyCode.begin(), bodyCode.end());

    // 6. Jump to startLabel
    code.push_back(TAC("jmp", "", "", startLabel));

    // 7. End label
    code.push_back(TAC("label", endLabel, "", ""));

    // Pop loop labels after processing
    AST::loopLabels.pop_back();

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    condition->resolveSymbol(symTab);
    symTab.enterScope();
    body->resolveSymbol(symTab);
    symTab.exitScope();
  }
};

//////////////////////////////////////////////////////////////////////////

class ForStmt : public Stmt {
public:
  std::unique_ptr<BlockItem> init; // Change to BlockItem
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Expr> inc;
  std::unique_ptr<Stmt> body;

  ForStmt(std::unique_ptr<BlockItem> init, std::unique_ptr<Expr> cond, std::unique_ptr<Expr> inc, std::unique_ptr<Stmt> body)
      : init(std::move(init)), cond(std::move(cond)), inc(std::move(inc)), body(std::move(body)) {}

  void print() {
    cout << "ForStmt: ";
    init->print();
    cond->print();
    inc->print();
    body->print();
  }

  StmtType getType() const override { return StmtType::FOR; }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    std::string condTemp;

    // 1. Create labels
    std::string startLabel = "L" + std::to_string(AST::tempVarCounter++);
    std::string incLabel = "L" + std::to_string(AST::tempVarCounter++);
    std::string endLabel = "L" + std::to_string(AST::tempVarCounter++);

    // Push loop labels: {continue -> incLabel, break -> endLabel}
    AST::loopLabels.push_back({incLabel, endLabel});
    
    // 2. Init
    auto initCode = init->generateTAC(tempVar);
    code.insert(code.end(), initCode.begin(), initCode.end());
    
    // 3. Start label
    code.push_back(TAC("label", startLabel, "", ""));
    
    // 4. Generate TAC for the condition, storing the result in condTemp
    auto condCode = cond->generateTAC(condTemp);
    code.insert(code.end(), condCode.begin(), condCode.end());
    
    // 5. Conditional jump: Jump to endLabel if condition is false (0)
    code.push_back(TAC("beqz", condTemp, endLabel, ""));
    
    // 6. Body
    auto bodyCode = body->generateTAC(tempVar);
    code.insert(code.end(), bodyCode.begin(), bodyCode.end());
    
    // 7. Increment
    code.push_back(TAC("label", incLabel, "", ""));
    auto incCode = inc->generateTAC(tempVar);
    code.insert(code.end(), incCode.begin(), incCode.end());
    
    // 8. Jump to startLabel
    code.push_back(TAC("jmp", "", "", startLabel));
    
    // 9. End label
    code.push_back(TAC("label", endLabel , "", ""));

    AST::loopLabels.pop_back();

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    symTab.enterScope();
    init->resolveSymbol(symTab);
    cond->resolveSymbol(symTab);
    inc->resolveSymbol(symTab);
    body->resolveSymbol(symTab);
    symTab.exitScope();
  }
};

//////////////////////////////////////////////////////////////////////////

class DoWhileStmt : public Stmt {
public:
  std::unique_ptr<Stmt> body;
  std::unique_ptr<Expr> cond;

  DoWhileStmt(std::unique_ptr<Stmt> body, std::unique_ptr<Expr> cond)
      : body(std::move(body)), cond(std::move(cond)) {}

  void print() {
    cout << "DoWhileStmt: ";
    body->print();
    cond->print();
  }

  StmtType getType() const override { return StmtType::DO_WHILE; }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    std::string condTemp;

    // 1. Create labels
    std::string startLabel = "L" + std::to_string(AST::tempVarCounter++);
    std::string condLabel = "L" + std::to_string(AST::tempVarCounter++);
    std::string endLabel = "L" + std::to_string(AST::tempVarCounter++);

    // Push loop labels: {continue -> condLabel, break -> endLabel}
    AST::loopLabels.push_back({condLabel, endLabel});

    // 2. Start label
    code.push_back(TAC("label", startLabel, "", ""));
    
    // 3. Body
    auto bodyCode = body->generateTAC(tempVar);
    code.insert(code.end(), bodyCode.begin(), bodyCode.end());
    
    // 4. Generate TAC for the condition, storing the result in condTemp
    auto condCode = cond->generateTAC(condTemp);
    code.push_back(TAC("label", condLabel, "", ""));
    code.insert(code.end(), condCode.begin(), condCode.end());
    
    // 5. Conditional jump: Jump to startLabel if condition is true (1)
    code.push_back(TAC("bnez", condTemp, startLabel, ""));

    // 6. End label
    code.push_back(TAC("label", endLabel, "", ""));

    AST::loopLabels.pop_back();

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    symTab.enterScope();
    body->resolveSymbol(symTab);
    cond->resolveSymbol(symTab);
    symTab.exitScope();
  }
};

//////////////////////////////////////////////////////////////////////////

class BreakStmt : public Stmt {
public:
  void print() { cout << "BreakStmt" << endl; }
  StmtType getType() const override { return StmtType::BREAK; }

  std::vector<TAC> generateTAC(std::string &tempVar) {
    std::vector<TAC> code;
  
    if (!AST::loopLabels.empty()) {
      code.push_back(TAC("jmp", "", "", AST::loopLabels.back().second)); // Jump to end label
    }else if(!AST::switchLabels.empty()){
      code.push_back(TAC("jmp", "", "", AST::switchLabels.back())); // Jump to end label
    }else {
      std::cerr << "Error: 'break' outside of loop" << std::endl;
    }
  
    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {}
};

//////////////////////////////////////////////////////////////////////////

class ContinueStmt : public Stmt {
public:
  void print() { cout << "ContinueStmt" << endl; }
  StmtType getType() const override { return StmtType::CONTINUE; }

  std::vector<TAC> generateTAC(std::string &tempVar) {
    std::vector<TAC> code;
  
    if (!AST::loopLabels.empty()) {
      code.push_back(TAC("jmp", "", "", AST::loopLabels.back().first)); // Jump to continue label
    } else {
      std::cerr << "Error: 'continue' outside of loop" << std::endl;
    }
  
    return code;
  }
  
  void resolveSymbol(SymbolTable &symTab) override {}
};

//////////////////////////////////////////////////////////////////////////

class SwitchStmt : public Stmt {
public:
  std::unique_ptr<Expr> expr;
  std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Stmt>>> cases;
  std::unique_ptr<Stmt> defaultCase;

  SwitchStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}

  void addCase(std::unique_ptr<Expr> caseExpr, std::unique_ptr<Stmt> caseStmt) {
    cases.push_back({std::move(caseExpr), std::move(caseStmt)});
  }

  void setDefault(std::unique_ptr<Stmt> defaultCase) {
    this->defaultCase = std::move(defaultCase);
  }

  void print() {
    cout << "SwitchStmt: ";
    expr->print();
    for (auto &case_ : cases) {
      case_.first->print();
      case_.second->print();
    }
    if (defaultCase) {
      defaultCase->print();
    }
  }

  StmtType getType() const override { return StmtType::SWITCH; }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    std::string exprTemp;

    // 1. Generate TAC for the switch expression
    auto exprCode = expr->generateTAC(exprTemp);
    code.insert(code.end(), exprCode.begin(), exprCode.end());

    // 2. Create labels for each case
    std::vector<std::string> caseLabels;
    for (int i = 0; i < cases.size(); i++) {
        caseLabels.push_back("L" + std::to_string(AST::tempVarCounter++));
    }
    std::string defaultLabel = defaultCase ? "L" + std::to_string(AST::tempVarCounter++) : ""; 
    std::string endLabel = "L" + std::to_string(AST::tempVarCounter++);

    AST::switchLabels.push_back(endLabel);

    // 3. Emit conditional jumps for each case
    for (int i = 0; i < cases.size(); i++) {
        std::string caseTemp;
        auto caseCode = cases[i].first->generateTAC(caseTemp);
        code.insert(code.end(), caseCode.begin(), caseCode.end());

        // If exprTemp == caseTemp, jump to corresponding case label
        code.push_back(TAC("beq", exprTemp, caseTemp, caseLabels[i]));
    }

    // 4. Jump to default case if it exists, otherwise jump to end
    if (!defaultLabel.empty()) {
        code.push_back(TAC("jmp", "", "", defaultLabel));
    } else {
        code.push_back(TAC("jmp", "", "", endLabel));
    }

    // 5. Emit TAC for each case statement
    for (int i = 0; i < cases.size(); i++) {
        code.push_back(TAC("label", caseLabels[i], "", ""));
        auto caseCode = cases[i].second->generateTAC(tempVar);
        code.insert(code.end(), caseCode.begin(), caseCode.end());

        // No automatic jump to endLabel to allow fall-through behavior
    }

    // 6. Default case
    if (!defaultLabel.empty()) {
        code.push_back(TAC("label", defaultLabel, "", ""));
        auto defaultCode = defaultCase->generateTAC(tempVar);
        code.insert(code.end(), defaultCode.begin(), defaultCode.end());
    }

    // 7. End label
    code.push_back(TAC("label", endLabel, "", ""));

    AST::switchLabels.pop_back();

    return code;
  }


  void resolveSymbol(SymbolTable &symTab) override {
    expr->resolveSymbol(symTab);
    symTab.enterScope();
    for (auto &case_ : cases) {
      case_.first->resolveSymbol(symTab);
      case_.second->resolveSymbol(symTab);
    }
    if (defaultCase) {
      defaultCase->resolveSymbol(symTab);
    }
    symTab.exitScope();
  }


};

//////////////////////////////////////////////////////////////////////////


// Variable declaration (e.g., `int x = 5;`)
class VarDecl : public Declaration {
  public:
    std::string name;
    std::unique_ptr<Expr> initializer;  // Optional initializer
  
    VarDecl(const std::string &name, std::unique_ptr<Expr> initializer = nullptr)
        : name(name), initializer(std::move(initializer)) {}
  
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
      if (!symTab.declareVariable(name)) {
        std::cerr << "ERROR: Redeclaration of variable '" << name << "'" << std::endl;
        exit(1);
      }
      if (initializer) {
        initializer->resolveSymbol(symTab);
      }
    }
};  

//////////////////////////////////////////////////////////////////////////

class FuncDecl : public Declaration {
public:
  std::string name;
  std::vector<std::string> params;
  std::unique_ptr<Block> body;

  FuncDecl(const std::string &name, std::vector<std::string> params, std::unique_ptr<Block> body)
      : name(name), params(std::move(params)), body(std::move(body)) {}

  void print() override {
    cout << "Function Declaration: " << name << "(";
    for (const auto &param : params) {
      cout << param << ", ";
    }
    cout << ")";
    if (body) {
      cout << endl;
      body->print();
    }
  }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    code.push_back(TAC("function", name, "", ""));
    
    // Emit TAC for function parameters
    for (const auto &param : params) {
      code.push_back(TAC("param", param, "", ""));
    }

    if (body) {
      auto bodyCode = body->generateTAC(tempVar);
      code.insert(code.end(), bodyCode.begin(), bodyCode.end());
    }
    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    // Ensure the function name is uniquely declared
    if (!symTab.declareFunction(name, params)) {
        std::cerr << "ERROR: Redeclaration of function '" << name << "'" << std::endl;
        exit(1);
    }
    // Only create a new scope if the function has a body
    if (body) {
        symTab.enterScope();
        
        for (const auto &param : params) {
            if (param == name) {
                std::cerr << "ERROR: Parameter '" << param << "' conflicts with function name '" << name << "'" << std::endl;
                exit(1);
            }

            if (!symTab.declareVariable(param)) {
                std::cerr << "ERROR: Redeclaration of parameter '" << param << "'" << std::endl;
                exit(1);
            }
        }

        body->resolveSymbol(symTab);
        symTab.exitScope();
    }
}

};

//////////////////////////////////////////////////////////////////////////

// class Func : public AST {
//   public:
//     std::string name;
//     std::unique_ptr<Block> body;
  
//     Func(std::string name, std::unique_ptr<Block> body)
//         : name(std::move(name)), body(std::move(body)) {}
  
//     void print() {
//       cout << "Function: " << name << endl;
//       cout << "Body: " << endl;
//       body->print();
//     }
  
//     std::vector<TAC> generateTAC(std::string &tempVar) override {
//       std::vector<TAC> code;
//       auto bodyCode = body->generateTAC(tempVar);
//       code.insert(code.end(), bodyCode.begin(), bodyCode.end());
//       return code;
//     }

//     void resolveSymbol(SymbolTable &symTab) override {
//       symTab.enterScope();
//       body->resolveSymbol(symTab);
//       symTab.exitScope();
//     }

//   };
  

class ASTProgram : public AST {
public:
  std::vector<std::unique_ptr<FuncDecl>> functions;

  void addFunction(std::unique_ptr<FuncDecl> func) {
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
    symTab.enterScope();
    for (auto &func : functions) {
      func->resolveSymbol(symTab);
    }
    symTab.exitScope();
  }
};