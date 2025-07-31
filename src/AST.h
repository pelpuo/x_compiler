#pragma once

#include "SymbolTable.h"
#include "TAC.h"
#include "Type.h"
#include "TypeChecker.h"
#include "lexer.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class AST;
class Expr;
class Factor;
class BinaryOp;
class UnaryOp;
class WithDecl;

class ConstInt;
class ConstLong;

using namespace std;

enum class StmtType {
  EXPR,
  RETURN,
  NULL_STMT,
  IF_STMT,
  BLOCK,
  WHILE,
  FOR,
  DO_WHILE,
  BREAK,
  CONTINUE,
  SWITCH,
  CASE,
  DEFAULT,
  DECL,
  FUNC_DECL,
  WITH_DECL
};

class ASTVisitor {
public:
  virtual void visit(AST &) {};
  virtual void visit(Expr &) {};
  virtual void visit(Factor &) = 0;
  virtual void visit(BinaryOp &) = 0;
  virtual void visit(UnaryOp &) = 0;
  virtual void visit(WithDecl &) = 0;

  virtual void visit(ConstInt &) = 0;
  virtual void visit(ConstLong &) = 0;
};

//////////////////////////////////////////////////////////////////////////

// Defining AST
class AST {
public:
  static int tempVarCounter; // Counter for temporary variables
  static std::vector<std::pair<std::string, std::string>>
      loopLabels; // Loop labels for break and continue
  static std::vector<std::string> switchLabels; // Switch labels for break
  virtual ~AST() = default;
  virtual std::vector<TAC> generateTAC(std::string &tempVar) = 0;
  virtual void resolveSymbol(SymbolTable &symTab) {}
  virtual Expr *typeCheck(SymbolTable &symTab) {}
};

//////////////////////////////////////////////////////////////////////////

// Defining Expr
class Expr : public AST {
public:
  virtual ~Expr() = default;

  std::unique_ptr<Type> expType = nullptr; // <--- NEW

  void setExprType(std::unique_ptr<Type> t) { expType = std::move(t); }

  Type *getExprType() const { return expType.get(); }

  virtual void print() = 0;
};

//////////////////////////////////////////////////////////////////////////

// Defining Block Items (eg Statements, Declarations)
class BlockItem : public AST {
public:
  virtual ~BlockItem() = default;
  virtual void print() = 0;
  virtual std::vector<TAC> generateTAC(std::string &tempVar) = 0;
  virtual void resolveSymbol(SymbolTable &symTab) = 0;
  virtual Expr *typeCheck(SymbolTable &symTab) = 0;
};

class Declaration : public BlockItem {
public:
  virtual ~Declaration() = default;
  virtual void print() = 0;
  virtual std::vector<TAC> generateTAC(std::string &tempVar) = 0;
  virtual void resolveSymbol(SymbolTable &symTab) = 0;
  virtual Expr *typeCheck(SymbolTable &symTab) = 0;
};

//////////////////////////////////////////////////////////////////////////

// Integer literal
// class IntLiteral : public Expr {
// public:
//   int value;
//   IntLiteral(int val) : value(val) {}
//   void print() { cout << "IntLiteral: " << value << endl; }

//   std::vector<TAC> generateTAC(std::string &tempVar) override {
//     std::vector<TAC> code;
//     tempVar = "t" + std::to_string(tempVarCounter++);
//     code.push_back(TAC("li", std::to_string(value), "", tempVar));
//     return code;
//   }

//   void resolveSymbol(SymbolTable &symTab) override {}

//   Expr *typeCheck(SymbolTable &symTab) override {
//     TypeChecker checker;
//     Expr *res = checker.typecheck(this, symTab);
//     this->expType = res->getExprType()->clone(); // optional, for safety
//   }
// };

//////////////////////////////////////////////////////////////////////////
class Constant : public Expr {
public:
  virtual ~Constant() = default;
  virtual void print() override = 0;
  virtual std::vector<TAC> generateTAC(std::string &tempVar) override = 0;
};

//////////////////////////////////////////////////////////////////////////
class ConstInt : public Constant {
public:
  int value;
  ConstInt(int val) : value(val) {}
  void print() { cout << "ConstInt: " << value << endl; }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    tempVar = "t" + std::to_string(tempVarCounter++);
    code.push_back(TAC("li", std::to_string(value), "int", tempVar));
    return code;
  }
  void resolveSymbol(SymbolTable &symTab) override {}

  Expr *typeCheck(SymbolTable &symTab) override {
    this->setExprType(std::make_unique<IntType>());
    return this;
  }
};

//////////////////////////////////////////////////////////////////////////

class ConstLong : public Constant {
public:
  int value;
  ConstLong(long val) : value(val) {}
  void print() { cout << "CosntIntL: " << value << endl; }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    tempVar = "t" + std::to_string(tempVarCounter++);
    code.push_back(TAC("li", std::to_string(value), "long", tempVar));
    return code;
  }
  void resolveSymbol(SymbolTable &symTab) override {}

  Expr *typeCheck(SymbolTable &symTab) override {
    this->setExprType(std::make_unique<LongType>());
    return this;
  }
};

//////////////////////////////////////////////////////////////////////////

class ArgList {
public:
  std::vector<std::unique_ptr<Expr>> args;

  void addArg(std::unique_ptr<Expr> arg) { args.push_back(std::move(arg)); }

  void print() {
    for (size_t i = 0; i < args.size(); ++i) {
      if (args[i])
        args[i]->print();
      if (i < args.size() - 1)
        cout << ", ";
    }
  }

  int size() const { return args.size(); }

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
// Static variable declaration (e.g., `static int x = 10;`)
class StaticVariable : public AST {
public:
  std::string name;
  bool isGlobal;
  std::unique_ptr<Type> type;
  StaticInit init; // your StaticInit variant: IntInit / LongInit

  StaticVariable(std::string n, bool g, std::unique_ptr<Type> t, StaticInit i)
      : name(std::move(n)), isGlobal(g), type(std::move(t)),
        init(std::move(i)) {}

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    std::string initStr = (init.kind == StaticInit::Kind::IntInit)
                              ? std::to_string(init.intVal)
                              : std::to_string(init.longVal);
    code.push_back(TAC("StaticVariable", name, isGlobal ? "1" : "0", initStr));
    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    SymbolInfo info;
    info.type = SymbolType::VARIABLE;
    info.storageClass = isGlobal ? StorageClass::EXTERN : StorageClass::STATIC;
    info.isGlobal = isGlobal;
    info.declaredType = std::move(type);
    info.initValue = {InitKind::Initial, init};

    if (!symTab.declareFileScopeVariable(name, std::move(info.declaredType),
                                         info.storageClass, info.initValue)) {
      std::cerr << "ERROR: Failed to declare static variable: " << name << "\n";
      exit(1);
    }
  }

  Expr *typeCheck(SymbolTable &symTab) override {
    // Type checking is not needed for static variables
    return nullptr;
  }
};

//////////////////////////////////////////////////////////////////////////

// Variable reference (e.g., `x`)
class Variable : public Expr {
public:
  std::string name;
  std::unique_ptr<Type> exprType = nullptr;

  Variable(const std::string &name) : name(name) {}
  void print() { cout << "Variable: " << name << endl; }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    if (!exprType) {
      std::cerr << "ERROR: exprType is null in Variable '" << name
                << "' during TAC generation.\n";
      exit(1);
    }

    std::vector<TAC> code;
    tempVar = "t" + std::to_string(tempVarCounter++);
    code.push_back(TAC("load", name, exprType->toString(), tempVar));
    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    // SymbolInfo *sym = symTab.resolve(name);
    if (!symTab.resolve(name)) {
      std::cerr << "ERROR: Undeclared variable '" << name << "'" << std::endl;
      exit(1);
    }

    exprType = symTab.resolve(name)->declaredType->clone();
  }

  Expr *typeCheck(SymbolTable &symTab) override {
    std::cout << "Typechecking Variable: " << name << std::endl;

    auto infoOpt = symTab.resolve(name);
    if (!infoOpt.has_value()) {
      std::cerr << "ERROR: Undeclared variable '" << name << "'\n";
      exit(1);
    }

    const SymbolInfo &info = *infoOpt;
    if (info.type == SymbolType::FUNCTION) {
      std::cerr << "ERROR: Function name used as variable: " << name << "\n";
      exit(1);
    }

    exprType = info.declaredType->clone();

    this->setExprType(exprType->clone());

    // std::cout << "Typechecked Variable: " << name
    //           << ", type: " << exprType->toString() << std::endl;

    return this;
  }
};

//////////////////////////////////////////////////////////////////////////

// Binary operation (e.g., `a + b`)
class BinaryOp : public Expr {
public:
  char op; // Operator like '+', '-', '*', '/'
  std::unique_ptr<Expr> left, right;
  std::unique_ptr<Type> expType = nullptr; // <--- NEW

  BinaryOp(char op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right,
           std::unique_ptr<Type> expType = nullptr)
      : op(op), left(std::move(left)), right(std::move(right)),
        expType(std::move(expType)) {}

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
#define TOKEN_TO_STRING(token, str)                                            \
  case TokenType::token:                                                       \
    opStr = str;                                                               \
    break;

    switch (op) {
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

  Expr *typeCheck(SymbolTable &symTab) override {
    std::cout << "Typechecking BinaryOp: " << TokenStr[(int)op] << std::endl;

    // Type check both children and reassign back into unique_ptrs
    Expr *lhsResult = left->typeCheck(symTab);
    if (lhsResult != left.get())
      left.reset(lhsResult);

    Expr *rhsResult = right->typeCheck(symTab);
    if (rhsResult != right.get())
      right.reset(rhsResult);

    // cout << "Left type: " << left->getExprType()->toString()
    //      << ", Right type: " << right->getExprType()->toString() <<
    //      std::endl;

    Expr *lhs = left.get();
    Expr *rhs = right.get();

    // Ensure types are available
    const Type *lt = lhs->getExprType();
    const Type *rt = rhs->getExprType();

    if (!lt || !rt) {
      std::cerr << "ERROR: Missing type in BinaryOp operands: ";
      if (!lt)
        std::cerr << "LHS type is null. ";
      if (!rt)
        std::cerr << "RHS type is null.";
      std::cerr << std::endl;
      exit(1);
    }

    // Logical ops always yield int
    if (op == TokenType::LOGICAL_AND || op == TokenType::LOGICAL_OR) {
      this->setExprType(std::make_unique<IntType>());
      return this;
    }

    const Type *common = TypeChecker::get_common_type(lt, rt);
    lhs = TypeChecker::convert_to(lhs, common);
    rhs = TypeChecker::convert_to(rhs, common);

    Expr *lhsCast = TypeChecker::convert_to(lhs, common);
    if (lhsCast != lhs)
      left.reset(lhsCast);

    Expr *rhsCast = TypeChecker::convert_to(rhs, common);
    if (rhsCast != rhs)
      right.reset(rhsCast);

    // Set result type based on operation
    switch (op) {
    case TokenType::PLUS:
    case TokenType::MINUS:
    case TokenType::MUL:
    case TokenType::DIV:
    case TokenType::MOD:
      this->setExprType(common->clone());
      break;
    default:
      this->setExprType(std::make_unique<IntType>());
      break;
    }

    return this;
  }
};
//////////////////////////////////////////////////////////////////////////

class UnaryOp : public Expr {
public:
  TokenType op;
  std::unique_ptr<Expr> operand;
  bool isPostfix;                          // [++ support]
  std::unique_ptr<Type> expType = nullptr; // <--- NEW

public:
  UnaryOp(TokenType op, std::unique_ptr<Expr> operand, bool isPostfix = false,
          std::unique_ptr<Type> type = nullptr)
      : op(op), operand(std::move(operand)), isPostfix(isPostfix),
        expType(std::move(type)) {} // [++ support]

  void print() {
    cout << "UnaryOp: " << TokenStr[(int)op] << " ";
    operand->print();
  }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    std::string exprTemp;

    // Generate TAC for the operand
    auto exprCode = operand->generateTAC(exprTemp);
    code.insert(code.end(), exprCode.begin(), exprCode.end());

    if (op == TokenType::INCREMENT || op == TokenType::DECREMENT) {
      std::string one = "1";
      std::string newTemp = "t" + std::to_string(tempVarCounter++);
      std::string opStr = (op == TokenType::INCREMENT) ? "+" : "-";

      // Apply increment or decrement: newTemp = exprTemp ± 1
      code.emplace_back(opStr, exprTemp, one, newTemp);

      // Store result back to the original location
      Variable *var = dynamic_cast<Variable *>(operand.get());
      if (!var) {
        std::cerr << "ERROR: ++/-- operand must be a variable" << std::endl;
        exit(1);
      }

      if (!expType) {
        std::cerr << "ERROR: UnaryOp used without typeCheck" << std::endl;
        exit(1);
      }

      code.emplace_back("store", newTemp, expType->toString(), var->name);

      // Result depends on whether it's prefix or postfix
      tempVar = isPostfix ? exprTemp : newTemp;
      return code;
    }

    // Regular unary operations
    tempVar = "t" + std::to_string(tempVarCounter++);
    std::string opStr;
    if (op == TokenType::MINUS)
      opStr = "NEG";
    else if (op == TokenType::COMPLEMENT)
      opStr = "~";
    else if (op == TokenType::LOGICAL_NOT) {
      opStr = "seq";
      code.emplace_back(opStr, exprTemp, "0", tempVar);
      return code;
    }

    code.emplace_back(opStr, exprTemp, "", tempVar);
    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    operand->resolveSymbol(symTab);
  }

  Expr *typeCheck(SymbolTable &symTab) override {
    std::cout << "Typechecking UnaryOp: " << TokenStr[(int)op] << std::endl;

    // Type check the operand
    Expr *innerResult = operand->typeCheck(symTab);
    if (!innerResult) {
      std::cerr << "ERROR: UnaryOp operand typeCheck returned null\n";
      exit(1);
    }

    // Update operand if typeCheck replaced it
    if (innerResult != operand.get()) {
      operand.reset(innerResult);
    }

    const Type *innerType = operand->getExprType();
    if (!innerType) {
      std::cerr << "ERROR: Missing exprType in UnaryOp operand\n";
      exit(1);
    }

    // Set result type
    if (op == TokenType::LOGICAL_NOT) {
      this->setExprType(std::make_unique<IntType>());
    } else {
      this->setExprType(innerType->clone());
    }

    return this;
  }
};
// Initialize static member

//////////////////////////////////////////////////////////////////////////

// Variable assignment (e.g., `x = 5;`)
class Assignment : public Expr {
public:
  std::unique_ptr<Expr> name;
  std::unique_ptr<Expr> value;
  std::unique_ptr<Type> expType = nullptr;

  Assignment(std::unique_ptr<Expr> name, std::unique_ptr<Expr> value,
             std::unique_ptr<Type> expType = nullptr)
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

    // FIX: Don't generate TAC for the LHS as an rvalue
    Variable *var = dynamic_cast<Variable *>(name.get());
    if (!var) {
      std::cerr << "ERROR: LHS of assignment must be a variable" << std::endl;
      exit(1);
    }
    nameTemp = var->name; // Just use the variable name

    // Generate TAC for the value
    auto valueCode = value->generateTAC(valueTemp);
    code.insert(code.end(), valueCode.begin(), valueCode.end());

    code.push_back(TAC("store", valueTemp, expType->toString(), nameTemp));

    tempVar = valueTemp;

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    name->resolveSymbol(symTab);
    value->resolveSymbol(symTab);
  }

  Expr *typeCheck(SymbolTable &symTab) override {
    std::cout << "Typechecking Assignment: " << std::endl;
    // Typecheck both sides
    Expr *lhsChecked = name->typeCheck(symTab);
    Expr *rhsChecked = value->typeCheck(symTab);

    if (!lhsChecked || !rhsChecked) {
      std::cerr << "ERROR: Assignment operands returned null expressions\n";
      exit(1);
    }

    const Type *lt = lhsChecked->getExprType();
    const Type *rt = rhsChecked->getExprType();

    if (!lt || !rt) {
      std::cerr << "ERROR: Assignment operands missing type info\n";
      if (!lt)
        std::cerr << "LHS type is null. ";
      if (!rt)
        std::cerr << "RHS type is null. ";
      std::cerr << std::endl;
      exit(1);
    }

    // Ensure LHS is an assignable l-value (e.g., variable)
    if (!dynamic_cast<Variable *>(lhsChecked)) {
      std::cerr
          << "ERROR: LHS of assignment must be a variable (not an r-value)\n";
      exit(1);
    }

    // Convert RHS to match LHS type
    Expr *convertedRHS = TypeChecker::convert_to(rhsChecked, lt);

    // Store updated expressions
    if (lhsChecked != name.get())
      name.reset(lhsChecked);
    if (convertedRHS != value.get())
      value.reset(convertedRHS);

    // Set type of the assignment expression
    this->setExprType(lt->clone());
    return this;
  }
};

//////////////////////////////////////////////////////////////////////////

// Compound assignment (e.g., `x += 5;`)
class CompoundAssignment : public Expr {
public:
  std::unique_ptr<Expr> left;
  TokenType op;
  std::unique_ptr<Expr> right;

  CompoundAssignment(TokenType op, std::unique_ptr<Expr> left,
                     std::unique_ptr<Expr> right)
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
    // auto nameCode = left->generateTAC(nameTemp);
    // code.insert(code.end(), nameCode.begin(), nameCode.end());

    Variable *var = dynamic_cast<Variable *>(left.get());
    if (!var) {
      std::cerr
          << "ERROR: Left-hand side of compound assignment must be a variable"
          << std::endl;
      exit(1);
    }
    nameTemp = var->name; // Use actual variable name

    // Generate TAC for the value
    auto valueCode = right->generateTAC(valueTemp);
    code.insert(code.end(), valueCode.begin(), valueCode.end());

    // Create a new temporary variable for the result
    resultTemp = "t" + std::to_string(tempVarCounter++);

    // Map TokenType to TAC operation
    std::string opStr;
#define TOKEN_TO_STRING(token, str)                                            \
  case TokenType::token:                                                       \
    opStr = str;                                                               \
    break;

    switch (op) {
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
    code.push_back(TAC("store", resultTemp, expType->toString(), nameTemp));

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    left->resolveSymbol(symTab);
    right->resolveSymbol(symTab);
  }

  Expr *typeCheck(SymbolTable &symTab) override {
    std::cout << "Typechecking CompoundAssignment: " << TokenStr[(int)op]
              << std::endl;

    // Typecheck both sides
    Expr *lhsResult = left->typeCheck(symTab);
    if (lhsResult != left.get())
      left.reset(lhsResult);

    Expr *rhsResult = right->typeCheck(symTab);
    if (rhsResult != right.get())
      right.reset(rhsResult);

    const Type *lt = left->getExprType();
    const Type *rt = right->getExprType();

    if (!lt || !rt) {
      std::cerr
          << "ERROR: Missing type info in compound assignment operands.\n";
      if (!lt)
        std::cerr << "LHS is null. ";
      if (!rt)
        std::cerr << "RHS is null. ";
      std::cerr << std::endl;
      exit(1);
    }

    // Ensure LHS is a variable
    if (!dynamic_cast<Variable *>(left.get())) {
      std::cerr << "ERROR: LHS of compound assignment must be a variable\n";
      exit(1);
    }

    // Convert RHS to LHS type
    Expr *converted = TypeChecker::convert_to(right.get(), lt);
    if (converted != right.get())
      right.reset(converted);

    // Result type of compound assignment is the LHS type
    this->setExprType(lt->clone());
    return this;
  }
};

//////////////////////////////////////////////////////////////////////////
class Cast : public Expr {
public:
  std::unique_ptr<Expr> expr;
  std::unique_ptr<Type> type; // e.g., "int", "long", etc.

  Cast(std::unique_ptr<Expr> expr, std::unique_ptr<Type> type)
      : expr(std::move(expr)), type(std::move(type)) {}

  void print() {
    cout << "Cast: (" << (type ? type->toString() : "null") << ") ";
    expr->print();
  }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    std::string exprTemp;

    // Generate TAC for the inner expression
    auto exprCode = expr->generateTAC(exprTemp);
    code.insert(code.end(), exprCode.begin(), exprCode.end());

    // Get source and destination types
    Type *fromType = expr->getExprType();
    Type *toType = this->type.get();

    // If type is unchanged, no cast needed
    if (fromType->getKind() == toType->getKind()) {
      tempVar = exprTemp;
      return code;
    }

    // Create a new temporary variable for the result
    tempVar = "t" + std::to_string(AST::tempVarCounter++);

    // Determine the cast instruction
    std::string inst;
    if (toType->getKind() == Type::Kind::LONG &&
        fromType->getKind() == Type::Kind::INT) {
      inst = "SignExtend";
    } else if (toType->getKind() == Type::Kind::INT &&
               fromType->getKind() == Type::Kind::LONG) {
      inst = "Truncate";
    } else {
      std::cerr << "ERROR: Unsupported cast from " << fromType->toString()
                << " to " << toType->toString() << std::endl;
      exit(1);
    }

    // Emit cast instruction
    code.emplace_back(inst, exprTemp, "", tempVar);

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    expr->resolveSymbol(symTab);
  }

  Expr *typeCheck(SymbolTable &symTab) override {
    std::cout << "Typechecking Cast: " << type->toString() << std::endl;
    // Typecheck the inner expression
    Expr *innerResult = expr->typeCheck(symTab);
    if (innerResult != expr.get()) {
      expr.reset(innerResult);
    }

    // The result of a cast has the type we cast to
    this->setExprType(type->clone());
    return this;
  }
};

//////////////////////////////////////////////////////////////////////////

class TernaryOp : public Expr {
public:
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Expr> trueExpr;
  std::unique_ptr<Expr> falseExpr;

  TernaryOp(std::unique_ptr<Expr> condition, std::unique_ptr<Expr> trueExpr,
            std::unique_ptr<Expr> falseExpr)
      : condition(std::move(condition)), trueExpr(std::move(trueExpr)),
        falseExpr(std::move(falseExpr)) {}

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
    code.push_back(
        TAC("move", trueTemp, "", tempVar));      // Store result in tempVar
    code.push_back(TAC("jmp", "", "", endLabel)); // Jump to end

    // 5. False expression
    code.push_back(TAC("label", falseLabel, "", ""));
    std::string falseTemp;
    auto falseCode = falseExpr->generateTAC(falseTemp);
    code.insert(code.end(), falseCode.begin(), falseCode.end());
    code.push_back(
        TAC("move", falseTemp, "", tempVar)); // Store result in tempVar

    // 6. End label
    code.push_back(TAC("label", endLabel, "", ""));

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    condition->resolveSymbol(symTab);
    trueExpr->resolveSymbol(symTab);
    falseExpr->resolveSymbol(symTab);
  }

  Expr *typeCheck(SymbolTable &symTab) override {
    std::cout << "Typechecking TernaryOp" << std::endl;
    TypeChecker checker;
    Expr *res = checker.typecheck(this, symTab);
    this->expType = res->getExprType()->clone(); // optional, for safety
  }
};

//////////////////////////////////////////////////////////////////////////

class FuncCall : public Expr {
public:
  std::string name;
  std::unique_ptr<ArgList> args;
  std::unique_ptr<Type> expType = nullptr; // <--- NEW

  FuncCall(const std::string &name, std::unique_ptr<ArgList> args,
           std::unique_ptr<Type> expType = nullptr)
      : name(name), args(std::move(args)), expType(std::move(expType)) {}

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
      std::cerr << "ERROR: Argument count mismatch for function '" << name
                << "'" << std::endl;
      exit(1);
    }
    if (args) {
      args->resolveSymbol(symTab);
    }
  }

  Expr *typeCheck(SymbolTable &symTab) override {
    std::cout << "Typechecking FuncCall: " << name << std::endl;

    auto infoOpt = symTab.resolve(name);
    if (!infoOpt.has_value()) {
      std::cerr << "ERROR: Call to undeclared function " << name << "\n";
      exit(1);
    }

    const SymbolInfo &info = *infoOpt;
    if (!info.declaredType ||
        info.declaredType->getKind() != Type::Kind::FUNCTION) {
      std::cerr << "ERROR: Symbol is not a function: " << name << "\n";
      exit(1);
    }

    const auto *fnType =
        static_cast<const FunctionType *>(info.declaredType.get());
    const auto &paramTypes = fnType->getParamTypes();

    if (paramTypes.size() != args->size()) {
      std::cerr << "ERROR: Function call argument count mismatch for function '"
                << name << "'\n";
      exit(1);
    }

    // Type check and coerce each argument
    for (size_t i = 0; i < paramTypes.size(); ++i) {
      Expr *arg = args->args[i]->typeCheck(symTab);
      const Type *expected = paramTypes[i].get();
      arg = TypeChecker::convert_to(arg, expected);
      if (arg != args->args[i].get()) {
        args->args[i].reset(arg);
      }
    }

    this->setExprType(fnType->getReturnType()->clone());
    return this;
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
      // std::cerr << "[DEBUG] Resolving BlockItem...\n";
      // item->print();
      item->resolveSymbol(symTab);
    }
    symTab.exitScope();
  }

  Expr *typeCheck(SymbolTable &symTab) override {
    symTab.enterScope();
    for (auto &item : items) {
      item->typeCheck(symTab);
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

  std::vector<TAC> generateTAC(std::string &tempvar) override {
    std::vector<TAC> code;
    std::string tempVar;

    // Generate TAC for the expression
    auto exprCode = expr->generateTAC(tempVar);
    code.insert(code.end(), exprCode.begin(), exprCode.end());

    // Suppress the EXPR line if it's a standalone postfix ++/-- expression
    if (auto *unOp = dynamic_cast<UnaryOp *>(expr.get())) {
      if (unOp->op == TokenType::INCREMENT ||
          unOp->op == TokenType::DECREMENT) {
        return code; // Don't emit EXPR for side-effect-only statements
      }
    }

    code.push_back(TAC("EXPR", tempVar, "", ""));
    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    expr->resolveSymbol(symTab);
  }

  Expr *typeCheck(SymbolTable &symTab) override { expr->typeCheck(symTab); }
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

  std::vector<TAC> generateTAC(string &tempvar) override {
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

  Expr *typeCheck(SymbolTable &symTab) override {
    cout << "Typechecking ReturnStmt" << endl;
    if (expr)
      expr->typeCheck(symTab);
  }
};

class NullStmt : public Stmt {
public:
  void print() { cout << "NullStmt" << endl; }
  StmtType getType() const override { return StmtType::NULL_STMT; }

  void resolveSymbol(SymbolTable &symTab) override {}

  std::vector<TAC> generateTAC(std::string &tempVar) override { return {}; }
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
    std::string thenLabel =
        "L" +
        std::to_string(AST::tempVarCounter++); // Label for the 'then' block
    std::string elseLabel = "";                // Initialize to empty string
    std::string endLabel = "L" + std::to_string(AST::tempVarCounter++);

    if (elseBlock) {
      elseLabel = "L" + std::to_string(AST::tempVarCounter++);
    }

    // 3. Conditional jump: Jump to elseLabel if condition is false (0)
    code.push_back(TAC("beqz", condTemp,
                       elseLabel.empty() ? endLabel : elseLabel,
                       "")); // Jump to end if no else

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

  Expr *typeCheck(SymbolTable &symTab) override {
    cout << "Typechecking IfStmt" << endl;
    if (condition)
      condition->typeCheck(symTab);
    symTab.enterScope();
    if (thenBlock)
      thenBlock->typeCheck(symTab);
    if (elseBlock)
      elseBlock->typeCheck(symTab);
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

  Expr *typeCheck(SymbolTable &symTab) override {
    cout << "Typechecking WhileStmt" << endl;
    symTab.enterScope();
    condition->typeCheck(symTab);
    body->typeCheck(symTab);
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

  ForStmt(std::unique_ptr<BlockItem> init, std::unique_ptr<Expr> cond,
          std::unique_ptr<Expr> inc, std::unique_ptr<Stmt> body)
      : init(std::move(init)), cond(std::move(cond)), inc(std::move(inc)),
        body(std::move(body)) {}

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
    code.push_back(TAC("label", endLabel, "", ""));

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

  Expr *typeCheck(SymbolTable &symTab) override {
    cout << "Typechecking ForStmt" << endl;
    symTab.enterScope();
    init->typeCheck(symTab);
    cond->typeCheck(symTab);
    inc->typeCheck(symTab);
    body->typeCheck(symTab);
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

  Expr *typeCheck(SymbolTable &symTab) override {
    cout << "Typechecking DoWhileStmt" << endl;
    symTab.enterScope();
    cond->typeCheck(symTab);
    body->typeCheck(symTab);
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
      code.push_back(TAC("jmp", "", "",
                         AST::loopLabels.back().second)); // Jump to end label
    } else if (!AST::switchLabels.empty()) {
      code.push_back(
          TAC("jmp", "", "", AST::switchLabels.back())); // Jump to end label
    } else {
      std::cerr << "Error: 'break' outside of loop" << std::endl;
    }

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {}

  Expr *typeCheck(SymbolTable &symTab) override {
    // Nothing to type check
  }
};

//////////////////////////////////////////////////////////////////////////

class ContinueStmt : public Stmt {
public:
  void print() { cout << "ContinueStmt" << endl; }
  StmtType getType() const override { return StmtType::CONTINUE; }

  std::vector<TAC> generateTAC(std::string &tempVar) {
    std::vector<TAC> code;

    if (!AST::loopLabels.empty()) {
      code.push_back(
          TAC("jmp", "", "",
              AST::loopLabels.back().first)); // Jump to continue label
    } else {
      std::cerr << "Error: 'continue' outside of loop" << std::endl;
    }

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {}

  Expr *typeCheck(SymbolTable &symTab) override {
    // Nothing to type check
  }
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
    std::string defaultLabel =
        defaultCase ? "L" + std::to_string(AST::tempVarCounter++) : "";
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

  Expr *typeCheck(SymbolTable &symTab) override {
    cout << "Type checking switch statement" << endl;
    if (expr)
      expr->typeCheck(symTab);
    symTab.enterScope();
    for (auto &case_ : cases) {
      case_.first->typeCheck(symTab);
      case_.second->typeCheck(symTab);
    }
    if (defaultCase)
      defaultCase->typeCheck(symTab);
    symTab.exitScope();
  }
};

//////////////////////////////////////////////////////////////////////////

// Variable declaration (e.g., `int x = 5;`)
class VarDecl : public Declaration {
public:
  std::string name;
  std::unique_ptr<Expr> initializer;
  std::unique_ptr<Type> type; // NEW: full type information
  StorageClass storage = StorageClass::NONE;
  TypeQualifier typeQualifier = TypeQualifier::NONE;

  VarDecl(const std::string &name, std::unique_ptr<Expr> initializer = nullptr,
          std::unique_ptr<Type> type = nullptr)
      : name(name), initializer(std::move(initializer)), type(std::move(type)) {
  }

  void print() override {
    cout << "Declaration: ";

    if (typeQualifier == TypeQualifier::CONST)
      cout << "const ";

    if (type) {
      switch (type->getKind()) {
      case Type::Kind::INT:
        cout << "int ";
        break;
      case Type::Kind::LONG:
        cout << "long ";
        break;
      default:
        cout << "unknown_type ";
        break;
      }
    }

    cout << name;

    if (initializer) {
      cout << " = ";
      initializer->print();
    }

    cout << endl;
  }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    tempVar = name;

    if (storage == StorageClass::STATIC) {
      std::string initVal = "0";
      if (initializer) {
        if (auto lit = dynamic_cast<ConstInt *>(initializer.get())) {
          initVal = std::to_string(lit->value);
        } else if (auto lit = dynamic_cast<ConstLong *>(initializer.get())) {
          initVal = std::to_string(lit->value);
        }
      }

      code.push_back(TAC("StaticVariable", name, type->toString(), initVal));
      return code;
    }

    if (storage == StorageClass::EXTERN) {
      return code;
    }

    if (initializer) {
      std::string initTemp;
      auto initCode = initializer->generateTAC(initTemp);
      code.insert(code.end(), initCode.begin(), initCode.end());
      std::string typeStr = "long";
      if (type && type->getKind() == Type::Kind::INT)
        typeStr = "int";
      code.push_back(TAC("store", initTemp, typeStr, name));
      // code.push_back(TAC("HERE", "", "", ""));
    }
    if (!initializer) {
      std::string tempReg = "t0"; // Or use a real temp
      std::string typeStr = "long";
      if (type && type->getKind() == Type::Kind::INT)
        typeStr = "int";

      code.push_back(TAC("li", "0", "", tempVar));
      code.push_back(TAC("store", tempVar, typeStr, name));
    }

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    if (!symTab.declareVariable(name, type->clone())) {
      std::cerr << "ERROR: Redeclaration of variable '" << name << "'"
                << std::endl;
      exit(1);
    }

    // std::cerr << "[DEBUG] Resolving symbol " << name << "\n";

    if (!symTab.resolve(name)) {
      std::cerr << "<SymbolResolution> ERROR: Undeclared variable '" << name
                << "'\n";
      exit(1);
    }

    if (initializer) {
      initializer->resolveSymbol(symTab);
      // initializer->typeCheck(symTab); // Ensure initializer is type-checked
    }
  }

  void setStorage(StorageClass s) { storage = s; }
  StorageClass getStorage() const { return storage; }

  void setType(std::unique_ptr<Type> t) { type = std::move(t); }
  const Type *getType() const { return type.get(); }

  void setQualifier(TypeQualifier q) { typeQualifier = q; }
  TypeQualifier getQualifier() const { return typeQualifier; }

  Expr *typeCheck(SymbolTable &symTab) override {
    cout << "Type checking variable declaration: " << name << endl;
    // Declare the variable first so it's available in its own initializer
    if (!symTab.declareVariable(name, type->clone())) {
      std::cerr << "ERROR: Redeclaration of variable '" << name << "'\n";
      exit(1);
    }

    // std::cerr << "[SymbolTable] Declaring var '" << name << "'\n";

    if (initializer) {
      initializer->typeCheck(symTab); // This should now internally set expType

      const Type *initType = initializer->getExprType();
      const Type *declType = type.get();

      if (!initType || !declType) {
        std::cerr << "ERROR: Missing type in initializer for '" << name
                  << "'\n";
        exit(1);
      }

      // Insert implicit cast if needed
      if (initType->getKind() != declType->getKind()) {
        auto casted =
            std::make_unique<Cast>(std::move(initializer), declType->clone());
        casted->setExprType(declType->clone());
        initializer = std::move(casted);
      }
    }
  }
};

//////////////////////////////////////////////////////////////////////////

class FuncDecl : public Declaration {
public:
  std::string name;
  std::vector<std::string> params;
  std::unique_ptr<Block> body;
  StorageClass storage = StorageClass::NONE; // Add this line
  std::unique_ptr<Type> type;

  FuncDecl(const std::string &name, std::vector<std::string> params,
           std::unique_ptr<Block> body, std::unique_ptr<Type> type)
      : name(name), params(std::move(params)), body(std::move(body)),
        type(std::move(type)) {}

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
    code.push_back(
        TAC("function", name, storage == StorageClass::EXTERN ? "0" : "1", ""));

    // Emit TAC for function parameters
    for (const auto &param : params) {
      code.push_back(TAC("param", param, "", ""));
    }

    if (body) {
      auto bodyCode = body->generateTAC(tempVar);
      code.insert(code.end(), bodyCode.begin(), bodyCode.end());
    }

    // code.push_back(TAC("RETURN", "0", "", ""));
    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    // Ensure the function name is uniquely declared
    if (!symTab.declareFunction(name, params, type->clone(), body != nullptr)) {
      std::cerr << "ERROR: Redeclaration of function '" << name << "'"
                << std::endl;
      exit(1);
    }
    // Only create a new scope if the function has a body
    if (body) {
      symTab.enterScope();

      for (const auto &param : params) {
        if (param == name) {
          std::cerr << "ERROR: Parameter '" << param
                    << "' conflicts with function name '" << name << "'"
                    << std::endl;
          exit(1);
        }

        if (!symTab.declareVariable(param, type->clone())) {
          std::cerr << "ERROR: Redeclaration of parameter '" << param << "'"
                    << std::endl;
          exit(1);
        }
      }

      body->resolveSymbol(symTab);
      symTab.exitScope();
    }
  }

  void setStorage(StorageClass s) { storage = s; }
  StorageClass getStorage() const { return storage; }
  void setType(std::unique_ptr<Type> t) { type = std::move(t); }
  const Type *getType() const { return type.get(); }

  Expr *typeCheck(SymbolTable &symTab) override {
    cout << "Type checking function declaration: " << name << endl;
    if (body) {
      symTab.enterScope();

      const auto *fnType = dynamic_cast<FunctionType *>(type.get());
      if (!fnType) {
        std::cerr << "ERROR: Function type is not FunctionType\n";
        exit(1);
      }

      const auto &paramTypes = fnType->getParamTypes();
      if (paramTypes.size() != params.size()) {
        std::cerr << "ERROR: Mismatch between parameter names and types\n";
        exit(1);
      }

      for (size_t i = 0; i < params.size(); ++i) {
        if (!symTab.declareVariable(params[i], paramTypes[i]->clone())) {
          std::cerr << "ERROR: Redeclaration of parameter '" << params[i]
                    << "'\n";
          exit(1);
        }
      }

      body->typeCheck(symTab);
      symTab.exitScope();
    }
  }
};

//////////////////////////////////////////////////////////////////////////

class ASTProgram : public AST {
public:
  std::vector<std::unique_ptr<FuncDecl>> functions;
  std::vector<std::unique_ptr<FuncDecl>> prototypes;

  void addFunction(std::unique_ptr<FuncDecl> func) {
    functions.push_back(std::move(func));
  }

  void addPrototype(std::unique_ptr<FuncDecl> proto) {
    prototypes.push_back(std::move(proto));
  }

  void print() {
    for (auto &proto : prototypes)
      proto->print();
    for (auto &func : functions)
      func->print();
  }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    // Do nothing or throw an error if this version should not be used
    std::cerr << "ERROR: ASTProgram::generateTAC without symbol table is not "
                 "supported.\n";
    return {};
  }

  std::vector<TAC> generateTAC(std::string &tempVar,
                               const SymbolTable &symTab) {
    std::vector<TAC> code;
    for (auto &func : functions) {
      std::string tempVar;
      auto funcCode = func->generateTAC(tempVar);
      code.insert(code.end(), funcCode.begin(), funcCode.end());
    }

    // Emit static variables from symbol table
    auto staticDefs = convertSymbolsToTAC(symTab);
    code.insert(code.end(), staticDefs.begin(), staticDefs.end());

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    symTab.enterScope();
    for (auto &proto : prototypes)
      proto->resolveSymbol(symTab); // Only declares symbol
    for (auto &func : functions)
      func->resolveSymbol(symTab);
    symTab.exitScope();
  }

  Expr *typeCheck(SymbolTable &symTab) override {
    symTab.enterScope();
    for (auto &proto : prototypes) {
      proto->typeCheck(symTab);
    }
    for (auto &func : functions) {
      func->typeCheck(symTab);
    }
    symTab.exitScope();
  }

  std::vector<TAC> convertSymbolsToTAC(const SymbolTable &symTab) {
    std::vector<TAC> TACdefs;

    // Iterate over the global scope
    for (const auto &[name, info] : symTab.globalScope) {
      if (info.type != SymbolType::VARIABLE)
        continue;

      if (info.storageClass != StorageClass::STATIC &&
          info.storageClass != StorageClass::EXTERN)
        continue; // Only care about static or extern variables

      const auto &init = info.initValue;
      if (init.kind == InitKind::NoInitializer)
        continue; // Not defined in this translation unit

      std::string initValStr = "0";
      if (init.kind == InitKind::Initial && init.value.has_value()) {
        const StaticInit &val = *init.value;
        if (val.kind == StaticInit::Kind::IntInit)
          initValStr = std::to_string(val.intVal);
        else
          initValStr = std::to_string(val.longVal);
      }

      TACdefs.emplace_back("StaticVariable", name, info.isGlobal ? "1" : "0",
                           initValStr);
    }

    return TACdefs;
  }
};