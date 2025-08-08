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
class Dereference;
class AddrOf;

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

  std::unique_ptr<Type> expType = nullptr; // existing type info
  bool isLValue = false;                   // NEW: lvalue tracking

  // ---- Type accessors ----
  void setExprType(std::unique_ptr<Type> t) { expType = std::move(t); }
  Type *getExprType() const { return expType.get(); }

  // ---- LValue accessors ----
  void setLValue(bool val) { isLValue = val; }
  bool getLValue() const { return isLValue; }

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
  void print() override { cout << "ConstInt: " << value << endl; }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    tempVar = "t" + std::to_string(tempVarCounter++);
    return {TAC("li", std::to_string(value), "int", tempVar)};
  }

  void resolveSymbol(SymbolTable &) override {}

  Expr *typeCheck(SymbolTable &) override {
    this->setExprType(std::make_unique<IntType>());
    return this;
  }
};

//////////////////////////////////////////////////////////////////////////

class ConstLong : public Constant {
public:
  long value;
  ConstLong(long val) : value(val) {}
  void print() override { cout << "ConstLong: " << value << endl; }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    tempVar = "t" + std::to_string(tempVarCounter++);
    return {TAC("li", std::to_string(value), "long", tempVar)};
  }

  void resolveSymbol(SymbolTable &) override {}

  Expr *typeCheck(SymbolTable &) override {
    this->setExprType(std::make_unique<LongType>());
    return this;
  }
};
//////////////////////////////////////////////////////////////////////////

class ConstDouble : public Constant {
public:
  double value;
  ConstDouble(double val) : value(val) {}
  void print() override { cout << "ConstDouble: " << value << endl; }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    tempVar = "t" + std::to_string(tempVarCounter++);
    return {TAC("li", std::to_string(value), "double", tempVar)};
  }

  void resolveSymbol(SymbolTable &) override {}

  Expr *typeCheck(SymbolTable &) override {
    this->setExprType(std::make_unique<DoubleType>());
    return this;
  }
};

//////////////////////////////////////////////////////////////////////////

class ConstUnsignedLong : public Constant {
public:
  unsigned long value;
  ConstUnsignedLong(unsigned long val) : value(val) {}
  void print() override { cout << "ConstUnsignedLong: " << value << endl; }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    tempVar = "t" + std::to_string(tempVarCounter++);
    return {TAC("li", std::to_string(value), "unsigned_long", tempVar)};
  }

  void resolveSymbol(SymbolTable &) override {}

  Expr *typeCheck(SymbolTable &) override {
    this->setExprType(std::make_unique<UnsignedLongType>());
    return this;
  }
};

//////////////////////////////////////////////////////////////////////////

class ConstUnsignedInt : public Constant {
public:
  unsigned int value;
  ConstUnsignedInt(unsigned int val) : value(val) {}
  void print() override { cout << "ConstUnsignedInt: " << value << endl; }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    tempVar = "t" + std::to_string(tempVarCounter++);
    return {TAC("li", std::to_string(value), "unsigned_int", tempVar)};
  }

  void resolveSymbol(SymbolTable &) override {}

  Expr *typeCheck(SymbolTable &) override {
    this->setExprType(std::make_unique<UnsignedIntType>());
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

    setLValue(true); // variables are lvalues

    return this;
  }
};

//////////////////////////////////////////////////////////////////////////

// Binary operation (e.g., `a + b`)
class BinaryOp : public Expr {
public:
  // char op; // Operator like '+', '-', '*', '/'
  TokenType op;
  std::unique_ptr<Expr> left, right;
  // std::unique_ptr<Type> expType = std::make_unique<IntType>(); // <--- NEW

  BinaryOp(TokenType op, std::unique_ptr<Expr> left,
           std::unique_ptr<Expr> right)
      : op(op), left(std::move(left)), right(std::move(right)) {}

  void print() {
    cout << "BinaryOp: ";
    left->print();
    cout << " " << TokenStr[(int)op] << " ";
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

  // In AST.h, replace the existing BinaryOp::typeCheck method
  Expr *typeCheck(SymbolTable &symTab) override {
    std::cout << "Typechecking BinaryOp: " << TokenStr[(int)op] << std::endl;

    // Type check children first, updating pointers as we go.
    Expr *lhsResult = left->typeCheck(symTab);
    if (lhsResult != left.get()) {
      left.reset(lhsResult);
    }

    Expr *rhsResult = right->typeCheck(symTab);
    if (rhsResult != right.get()) {
      right.reset(rhsResult);
    }

    const Type *lt = left->getExprType();
    const Type *rt = right->getExprType();

    if (!lt || !rt) {
      std::cerr << "ERROR: Missing type in BinaryOp operands." << std::endl;
      exit(1);
    }

    if (op == TokenType::LOGICAL_AND || op == TokenType::LOGICAL_OR) {
      this->setExprType(std::make_unique<IntType>());
      return this;
    }

    // Handle Equal and NotEqual operators with pointer-aware logic
    if (op == TokenType::EQUAL_EQUAL || op == TokenType::NOT_EQUAL) {
      const Type *commonType = nullptr;
      bool eitherIsPointer = lt->isPointerType() || rt->isPointerType();

      if (eitherIsPointer) {
        // Use get_common_pointer_type which expects Expr*, pass your expr
        // pointers
        commonType =
            TypeChecker::get_common_pointer_type(left.get(), right.get());
      } else {
        commonType = TypeChecker::get_common_type(lt, rt);
      }

      // Convert operands to common type
      left.reset(TypeChecker::convert_to(left.release(), commonType));
      right.reset(TypeChecker::convert_to(right.release(), commonType));

      // Equality comparison result is int
      this->setExprType(std::make_unique<IntType>());
      return this;
    }

    // Reject arithmetic * / % on pointers
    if ((op == TokenType::MUL || op == TokenType::DIV ||
         op == TokenType::MOD) &&
        (lt->isPointerType() || rt->isPointerType())) {
      std::cerr << "ERROR: Cannot apply arithmetic operator '"
                << TokenStr[(int)op] << "' to pointer types\n";
      exit(1);
    }

    // For other ops, disallow pointer operands for now or handle if you
    // implement pointer arithmetic later
    if (lt->isPointerType() || rt->isPointerType()) {
      std::cerr << "ERROR: Unsupported binary operator '" << TokenStr[(int)op]
                << "' for pointer types\n";
      exit(1);
    }

    const Type *common = TypeChecker::get_common_type(lt, rt);

    // This is the fix: Release ownership, convert, and reset the unique_ptr.
    left.reset(TypeChecker::convert_to(left.release(), common));
    right.reset(TypeChecker::convert_to(right.release(), common));

    // Set the result type for the operation itself.
    switch (op) {
    case TokenType::MOD:
    case TokenType::BITWISE_AND:
    case TokenType::BITWISE_OR:
    case TokenType::BITWISE_XOR:
    case TokenType::LEFT_SHIFT:
    case TokenType::RIGHT_SHIFT:
      if (common->getKind() == Type::Kind::DOUBLE) {
        std::cerr
            << "ERROR: Cannot apply modulus operator (%) to type 'double'\n";
        exit(1);
      }
      this->setExprType(common->clone());
      break;
    case TokenType::PLUS:
    case TokenType::MINUS:
    case TokenType::MUL:
    case TokenType::DIV:
      this->setExprType(common->clone());
      break;
    default: // Relational and equality operators result in int.
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
  bool isPostfix; // [++ support]
  // std::unique_ptr<Type> expType = std::make_unique<IntType>(); // <--- NEW
  // std::unique_ptr<Type> expType; // <--- NEW

public:
  UnaryOp(TokenType op, std::unique_ptr<Expr> operand, bool isPostfix = false)
      : op(op), operand(std::move(operand)), isPostfix(isPostfix) {
  } // [++ support]

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

      if (!this->operand->expType) {
        std::cerr << "ERROR: UnaryOp used without typeCheck" << std::endl;
        exit(1);
      }

      code.emplace_back("store", newTemp, this->expType->toString(), var->name);

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

    // Type check the operand first
    Expr *innerResult = operand->typeCheck(symTab);
    if (!innerResult) {
      std::cerr << "ERROR: UnaryOp operand typeCheck returned null\n";
      exit(1);
    }

    // Replace operand if it was transformed during typeCheck
    if (innerResult != operand.get()) {
      operand.reset(innerResult);
    }

    const Type *innerType = operand->getExprType();
    if (!innerType) {
      std::cerr << "ERROR: Missing exprType in UnaryOp operand\n";
      exit(1);
    }

    switch (op) {
    case TokenType::COMPLEMENT:
      if (innerType->getKind() == Type::Kind::DOUBLE) {
        std::cerr
            << "ERROR: Cannot apply bitwise complement (~) to type 'double'\n";
        exit(1);
      } else if (innerType->getKind() == Type::Kind::POINTER) {
        std::cerr
            << "ERROR: Cannot apply bitwise complement (~) to type 'pointer'\n";
        exit(1);
      }
      this->setExprType(innerType->clone());
      break;
    case TokenType::MINUS:
      if (innerType->getKind() == Type::Kind::POINTER) {
        std::cerr
            << "ERROR: Cannot apply bitwise complement (~) to type 'pointer'\n";
        exit(1);
      }
      // Propagate operand's type
      cout << "UnaryOp: " << TokenStr[(int)op]
           << " with operand type: " << innerType->toString() << std::endl;
      this->setExprType(innerType->clone());
    case TokenType::INCREMENT:
    case TokenType::DECREMENT:
      // Propagate operand's type
      cout << "UnaryOp: " << TokenStr[(int)op]
           << " with operand type: " << innerType->toString() << std::endl;
      this->setExprType(innerType->clone());
      break;

    case TokenType::LOGICAL_NOT:
      // Logical NOT always yields an int
      this->setExprType(std::make_unique<IntType>());
      break;

    default:
      std::cerr << "ERROR: Unsupported unary operator in typeCheck: "
                << TokenStr[(int)op] << "\n";
      exit(1);
    }

    return this;
  }
};
// Initialize static member

//////////////////////////////////////////////////////////////////////////

class Dereference : public Expr {
public:
  std::unique_ptr<Expr> operand;
  Dereference(std::unique_ptr<Expr> operand) : operand(std::move(operand)) {}
  void print() {
    cout << "Dereference: *";
    operand->print();
  }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    std::string ptrTemp;

    // 1. Generate TAC for the operand to get the pointer address into ptrTemp.
    auto operandCode = operand->generateTAC(ptrTemp);
    code.insert(code.end(), operandCode.begin(), operandCode.end());

    // 2. Create a new temporary variable for the dereferenced value.
    tempVar = "t" + std::to_string(tempVarCounter++);

    // 3. Emit the "load_ptr" instruction to read from the address in ptrTemp.
    code.push_back(TAC("load_ptr", ptrTemp, "", tempVar));

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    operand->resolveSymbol(symTab);
  }

  Expr *typeCheck(SymbolTable &symTab) override {
    std::cout << "Typechecking Dereference" << std::endl;

    // 1. Type-check the operand first
    Expr *checkedOperand = operand->typeCheck(symTab);
    if (checkedOperand != operand.get()) {
      operand.reset(checkedOperand);
    }

    // 2. Get operand type
    const Type *operandType = operand->getExprType();
    if (!operandType) {
      std::cerr << "ERROR: Dereference operand has no type\n";
      exit(1);
    }

    // 3. Require that the operand be a pointer type
    if (operandType->getKind() != Type::Kind::POINTER) {
      std::cerr << "ERROR: Cannot dereference non-pointer type '"
                << operandType->toString() << "'\n";
      exit(1);
    }

    // 4. Set the resulting expression type to the pointed-to type
    const PointerType *ptrType = static_cast<const PointerType *>(operandType);
    if (!ptrType->referencedType) {
      std::cerr << "ERROR: Pointer type missing referenced type\n";
      exit(1);
    }
    this->setExprType(ptrType->referencedType->clone());
    setLValue(true);

    return this;
  }
};

//////////////////////////////////////////////////////////////////////////

class AddrOf : public Expr {
public:
  std::unique_ptr<Expr> operand;
  AddrOf(std::unique_ptr<Expr> operand) : operand(std::move(operand)) {}
  void print() {
    cout << "AddrOf: &";
    operand->print();
  }

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;

    // Check if the operand is a Dereference operation (&*ptr)
    if (auto *deref = dynamic_cast<Dereference *>(operand.get())) {
      // The & and * cancel out. Just generate the code for the inner pointer
      // expression.
      return deref->operand->generateTAC(tempVar);

      // Check if the operand is a simple variable (&var)
    } else if (auto *var = dynamic_cast<Variable *>(operand.get())) {
      tempVar = "t" + std::to_string(AST::tempVarCounter++);
      // Use the "getAddress" instruction.
      code.push_back(TAC("getAddress", var->name, "", tempVar));
      return code;

    } else {
      // Handling more complex cases like &a[i] or &s.m would go here.
      // For now, we can report an error for unsupported cases.
      std::cerr << "ERROR: Address-of operator can only be applied to "
                   "variables or dereferences."
                << std::endl;
      exit(1);
    }
  }

  void resolveSymbol(SymbolTable &symTab) override {
    operand->resolveSymbol(symTab);
  }

  Expr *typeCheck(SymbolTable &symTab) {
    Expr *checkedOperand = operand->typeCheck(symTab);
    if (checkedOperand != operand.get())
      operand.reset(checkedOperand);

    if (!operand->getLValue()) {
      std::cerr << "ERROR: Cannot take address of non-lvalue\n";
      exit(1);
    }

    setExprType(std::make_unique<PointerType>(operand->getExprType()->clone()));
    setLValue(false); // &expr is not an lvalue
    return this;
  }
};

//////////////////////////////////////////////////////////////////////////

// Variable assignment (e.g., `x = 5;`)
class Assignment : public Expr {
public:
  std::unique_ptr<Expr> name;
  std::unique_ptr<Expr> value;
  // std::unique_ptr<Type> expType;

  Assignment(std::unique_ptr<Expr> name, std::unique_ptr<Expr> value)
      : name(std::move(name)), value(std::move(value)) {}

  void print() {
    cout << "AssignStmt: ";
    name->print();
    cout << " = ";
    value->print();
  }

  // std::vector<TAC> generateTAC(std::string &tempVar) override {
  //   std::vector<TAC> code;
  //   std::string nameTemp, valueTemp;

  //   // FIX: Don't generate TAC for the LHS as an rvalue
  //   Variable *var = dynamic_cast<Variable *>(name.get());
  //   if (!var) {
  //     std::cerr << "ERROR: LHS of assignment must be a variable" <<
  //     std::endl; exit(1);
  //   }
  //   nameTemp = var->name; // Just use the variable name

  //   // Generate TAC for the value
  //   auto valueCode = value->generateTAC(valueTemp);
  //   code.insert(code.end(), valueCode.begin(), valueCode.end());

  //   // code.push_back(TAC("store", valueTemp, "int", nameTemp));
  //   code.push_back(
  //       TAC("store", valueTemp, this->expType->toString(), nameTemp));

  //   tempVar = valueTemp;

  //   return code;
  // }

  // In AST.h

  std::vector<TAC> generateTAC(std::string &tempVar) override {
    std::vector<TAC> code;
    std::string valueTemp;

    // First, always generate TAC for the right-hand side value.
    auto valueCode = value->generateTAC(valueTemp);
    code.insert(code.end(), valueCode.begin(), valueCode.end());

    // Check if the left-hand side is a dereference (*ptr = ...)
    if (auto *deref = dynamic_cast<Dereference *>(name.get())) {
      std::string ptrTemp;

      // Generate TAC for the expression that gives us the pointer address
      // (*ptr).
      auto ptrCode = deref->operand->generateTAC(ptrTemp);
      code.insert(code.end(), ptrCode.begin(), ptrCode.end());

      // Emit "store_ptr" to write the value into the memory location.
      code.push_back(TAC("store_ptr", valueTemp, "", ptrTemp));

      // Otherwise, it's a regular variable assignment (var = ...)
    } else if (auto *var = dynamic_cast<Variable *>(name.get())) {
      // This is the existing logic for variable assignment.
      code.push_back(
          TAC("store", valueTemp, this->expType->toString(), var->name));

    } else {
      std::cerr << "ERROR: Left-hand side of assignment is not a valid lvalue."
                << std::endl;
      exit(1);
    }

    tempVar = valueTemp; // The result of an assignment is the assigned value.
    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    name->resolveSymbol(symTab);
    value->resolveSymbol(symTab);
  }

  // In AST.h, replace the existing Assignment::typeCheck method
  Expr *typeCheck(SymbolTable &symTab) override {
    std::cout << "Typechecking Assignment: " << std::endl;

    Expr *lhsChecked = name->typeCheck(symTab);
    if (!lhsChecked->getLValue()) {
      std::cerr << "ERROR: Left-hand side of assignment is not an lvalue\n";
      exit(1);
    }

    if (lhsChecked != name.get()) {
      name.reset(lhsChecked);
    }

    Expr *rhsChecked = value->typeCheck(symTab);
    if (rhsChecked != value.get()) {
      value.reset(rhsChecked);
    }

    const Type *lt = name->getExprType();
    const Type *rt = value->getExprType();

    if (!lt || !rt) {
      std::cerr << "ERROR: Assignment operands missing type info." << std::endl;
      exit(1);
    }
    // if (!dynamic_cast<Variable *>(name.get())) {
    //   std::cerr << "ERROR: LHS of assignment must be a variable." << std::endl;
    //   exit(1);
    // }

    // Simplify the conversion to a direct release-and-reset.
    value.reset(TypeChecker::convert_by_assignment(value.release(), lt));

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

  // In AST.h, replace the existing CompoundAssignment::typeCheck method
  Expr *typeCheck(SymbolTable &symTab) override {
    std::cout << "Typechecking CompoundAssignment: " << TokenStr[(int)op]
              << std::endl;

    // Typecheck both sides
    Expr *lhsResult = left->typeCheck(symTab);
    if (lhsResult != left.get()) {
      left.reset(lhsResult);
    }

    Expr *rhsResult = right->typeCheck(symTab);
    if (rhsResult != right.get()) {
      right.reset(rhsResult);
    }

    const Type *lt = left->getExprType();
    const Type *rt = right->getExprType();

    if (!lt || !rt) {
      std::cerr
          << "ERROR: Missing type info in compound assignment operands.\n";
      exit(1);
    }
    if (!dynamic_cast<Variable *>(left.get())) {
      std::cerr << "ERROR: LHS of compound assignment must be a variable\n";
      exit(1);
    }

    // This is the fix: Release ownership, convert, and reset the unique_ptr.
    right.reset(TypeChecker::convert_to(right.release(), lt));

    // Result type of compound assignment is the LHS type.
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

    if (!fromType || !toType) {
      std::cerr << "ERROR: Cast has null type\n";
      exit(1);
    }

    // If type is unchanged, no cast needed
    if (fromType->getKind() == toType->getKind()) {
      tempVar = exprTemp;
      return code;
    }

    // Create a new temporary variable for the result
    tempVar = "t" + std::to_string(AST::tempVarCounter++);

    std::string inst;
    Type::Kind fromKind = fromType->getKind();
    Type::Kind toKind = toType->getKind();

    if (fromKind == Type::Kind::DOUBLE) {
      if (toKind == Type::Kind::INT) {
        inst = "DoubleToInt";
      } else if (toKind == Type::Kind::UNSIGNED_INT) {
        inst = "DoubleToUInt";
      } else {
        std::cerr << "ERROR: Unsupported cast from double to "
                  << toType->toString() << "\n";
        exit(1);
      }
    } else if (toKind == Type::Kind::DOUBLE) {
      if (fromKind == Type::Kind::INT) {
        inst = "IntToDouble";
      } else if (fromKind == Type::Kind::UNSIGNED_INT) {
        inst = "UIntToDouble";
      } else {
        std::cerr << "ERROR: Unsupported cast from " << fromType->toString()
                  << " to double\n";
        exit(1);
      }
    } else {
      // Fallback to truncation/widening logic
      int fromSize = TypeChecker::getTypeSize(fromType);
      int toSize = TypeChecker::getTypeSize(toType);

      if (fromSize == toSize) {
        inst = "Copy";
      } else if (toSize < fromSize) {
        inst = "Truncate";
      } else {
        bool fromSigned =
            (fromKind == Type::Kind::INT || fromKind == Type::Kind::LONG);
        inst = fromSigned ? "SignExtend" : "ZeroExtend";
      }
    }

    code.emplace_back(inst, exprTemp, "", tempVar);
    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    expr->resolveSymbol(symTab);
  }

  Expr *typeCheck(SymbolTable &symTab) override {
    std::cout << "Typechecking Cast: " << type->toString() << std::endl;

    // Type check the inner expression
    Expr *innerResult = expr->typeCheck(symTab);
    if (innerResult != expr.get()) {
      expr.reset(innerResult);
    }

    const Type *srcType = expr->getExprType();
    const Type *destType = type.get();

    if (!srcType || !destType) {
      std::cerr << "ERROR: Missing type in Cast\n";
      exit(1);
    }

    // Disallow pointer <-> double casts
    if ((srcType->isPointerType() && destType->isDouble()) ||
        (srcType->isDouble() && destType->isPointerType())) {
      std::cerr << "ERROR: Cannot cast between pointer and double types\n";
      exit(1);
    }

    // Otherwise, allow cast and set type to destination type
    this->setExprType(destType->clone());

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

    // Type check subexpressions
    condition = std::unique_ptr<Expr>(condition->typeCheck(symTab));
    trueExpr = std::unique_ptr<Expr>(trueExpr->typeCheck(symTab));
    falseExpr = std::unique_ptr<Expr>(falseExpr->typeCheck(symTab));

    const Type *condType = condition->getExprType();
    if (!condType) {
      std::cerr << "ERROR: Ternary condition must be of type int\n";
      exit(1);
    }

    const Type *type1 = trueExpr->getExprType();
    const Type *type2 = falseExpr->getExprType();
    if (!type1 || !type2) {
      std::cerr << "ERROR: Ternary branches must have valid types\n";
      exit(1);
    }

    // Compute common type
    const Type *common = TypeChecker::get_common_type(type1, type2);
    if (!common) {
      std::cerr
          << "ERROR: Cannot determine common type in ternary expression\n";
      exit(1);
    }

    // Perform implicit conversions if necessary
    if (type1->getKind() != common->getKind()) {
      trueExpr.reset(new Cast(std::move(trueExpr), common->clone()));
    }

    if (type2->getKind() != common->getKind()) {
      falseExpr.reset(new Cast(std::move(falseExpr), common->clone()));
    }

    this->expType = common->clone();
    return this;
  }
};

//////////////////////////////////////////////////////////////////////////

class FuncCall : public Expr {
public:
  std::string name;
  std::unique_ptr<ArgList> args;
  std::unique_ptr<Type> expType; // <--- NEW

  FuncCall(const std::string &name, std::unique_ptr<ArgList> args)
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
      arg = TypeChecker::convert_by_assignment(arg, expected);
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

  // In AST.h, replace the existing Block::typeCheck method
  Expr *typeCheck(SymbolTable &symTab) override {
    symTab.enterScope();
    for (auto &item : items) {
      item->typeCheck(
          symTab); // Call typeCheck, discard the (now always) null pointer
    }
    symTab.exitScope();
    return nullptr; // Block itself doesn't return a value
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

    // code.push_back(TAC("EXPR", tempVar, "", ""));
    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    expr->resolveSymbol(symTab);
  }

  // Expr *typeCheck(SymbolTable &symTab) override {
  //   return expr->typeCheck(symTab);
  // }
  // In AST.h, replace the existing ExprStmt::typeCheck method
  Expr *typeCheck(SymbolTable &symTab) override {
    if (expr) {
      Expr *newExpr = expr->typeCheck(symTab);
      if (newExpr != expr.get()) {
        expr.reset(newExpr);
      }
    }
    return nullptr; // Statements should not return expressions
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

  // In AST.h, replace the existing ReturnStmt::typeCheck method
  Expr *typeCheck(SymbolTable &symTab) override {
    cout << "Typechecking ReturnStmt" << endl;

    const Type *funcRetType = symTab.currentFunctionReturnType;

    if (!funcRetType) {
      std::cerr << "Return statement outside of function\n";
      exit(1);
    }

    if (expr) {
      // Capture the result and update the pointer if it changed.
      Expr *newExpr = expr->typeCheck(symTab);
      if (newExpr != expr.get())
        expr.reset(newExpr);
      expr.reset(
          TypeChecker::convert_by_assignment(expr.release(), funcRetType));
    }
    return nullptr; // A statement does not return an expression.
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
    if (condition) {
      Expr *condResult = condition->typeCheck(symTab);
      if (condResult != condition.get()) {
        condition.reset(condResult);
      }
    }
    symTab.enterScope();
    if (thenBlock)
      thenBlock->typeCheck(symTab);
    if (elseBlock)
      elseBlock->typeCheck(symTab);
    symTab.exitScope();

    return nullptr; // IfStmt doesn't return a value
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
    if (condition) {
      Expr *condResult = condition->typeCheck(symTab);
      if (condResult != condition.get()) {
        condition.reset(condResult);
      }
    }
    symTab.enterScope();
    body->typeCheck(symTab);
    symTab.exitScope();

    return nullptr; // WhileStmt doesn't return a value
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

    if (init)
      init->typeCheck(symTab);

    if (cond) {
      Expr *condResult = cond->typeCheck(symTab);
      if (condResult != cond.get()) {
        cond.reset(condResult);
      }
    }

    if (inc) {
      Expr *incResult = inc->typeCheck(symTab);
      if (incResult != inc.get()) {
        inc.reset(incResult);
      }
    }

    if (body)
      body->typeCheck(symTab);

    symTab.exitScope();
    return nullptr;
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
    if (cond) {
      Expr *condResult = cond->typeCheck(symTab);
      if (condResult != cond.get()) {
        cond.reset(condResult);
      }
    }
    body->typeCheck(symTab);
    symTab.exitScope();

    return nullptr; // DoWhileStmt doesn't return a value
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
    return nullptr; // BreakStmt doesn't return a value
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
    return nullptr; // ContinueStmt doesn't return a value
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
    return nullptr; // SwitchStmt doesn't return a value
  }
};

//////////////////////////////////////////////////////////////////////////

// Variable declaration (e.g., `int x = 5;`)
class VarDecl : public Declaration {
public:
  std::string name;
  std::unique_ptr<Expr> initializer;
  std::unique_ptr<Type> expType; // NEW: full type information
  StorageClass storage = StorageClass::NONE;
  TypeQualifier typeQualifier = TypeQualifier::NONE;

  VarDecl(const std::string &name, std::unique_ptr<Expr> initializer = nullptr,
          std::unique_ptr<Type> expType = nullptr)
      : name(name), initializer(std::move(initializer)),
        expType(std::move(expType)) {}

  void print() override {
    cout << "Declaration: ";

    if (typeQualifier == TypeQualifier::CONST)
      cout << "const ";

    if (expType) {
      switch (expType->getKind()) {
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

      code.push_back(TAC("StaticVariable", name, expType->toString(), initVal));
      return code;
    }

    if (storage == StorageClass::EXTERN) {
      return code;
    }

    if (initializer) {
      std::string initTemp;
      auto initCode = initializer->generateTAC(initTemp);
      code.insert(code.end(), initCode.begin(), initCode.end());

      code.push_back(TAC("store", initTemp, expType->toString(), name));
      // code.push_back(TAC("HERE", "", "", ""));
    }
    if (!initializer) {
      std::string tempReg = "t0"; // Or use a real temp

      code.push_back(TAC("li", "0", "", tempVar));
      code.push_back(TAC("store", tempVar, expType->toString(), name));
    }

    return code;
  }

  void resolveSymbol(SymbolTable &symTab) override {
    if (!symTab.declareVariable(name, expType->clone())) {
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

  void setType(std::unique_ptr<Type> t) { expType = std::move(t); }
  const Type *getType() const { return expType.get(); }

  void setQualifier(TypeQualifier q) { typeQualifier = q; }
  TypeQualifier getQualifier() const { return typeQualifier; }

  Expr *typeCheck(SymbolTable &symTab) override {
    cout << "Type checking variable declaration: " << name << endl;

    // Check for static initializers before declaring the variable
    if (storage == StorageClass::STATIC && initializer) {
      bool isNullPtr = expType->isPointerType() &&
                       TypeChecker::is_null_pointer_constant(initializer.get());

      // --- START: NEW LOGIC FOR STATIC POINTERS ---
      if (expType->isPointerType()) {
        if (isNullPtr) {
          // This is the supported case: static int *p = 0;
          // We can proceed. The symbol table logic will handle storing it
          // as ULongInit(0).
        } else if (dynamic_cast<AddrOf *>(initializer.get())) {
          // This is the unsupported case: static int *p = &a;
          std::cerr << "ERROR: Initializer for static pointer '" << name
                    << "' is not a constant null pointer." << std::endl;
          exit(1);
        } else if (!dynamic_cast<Constant *>(initializer.get())) {
          // Reject any other non-constant initializer for static variables
          std::cerr << "ERROR: Initializer for static variable '" << name
                    << "' is not a constant expression." << std::endl;
          exit(1);
        }
      }
      // --- END: NEW LOGIC FOR STATIC POINTERS ---
    }
    // Declare the variable first so it's available in its own initializer
    if (!symTab.declareVariable(name, expType->clone())) {
      std::cerr << "ERROR: Redeclaration of variable '" << name << "'\n";
      exit(1);
    }

    // std::cerr << "[SymbolTable] Declaring var '" << name << "'\n";

    if (initializer) {
      initializer->typeCheck(symTab); // This should now internally set expType

      const Type *initType = initializer->getExprType();
      const Type *declType = expType.get();

      if (!initType || !declType) {
        std::cerr << "ERROR: Missing type in initializer for '" << name
                  << "'\n";
        exit(1);
      }

      // Insert implicit cast if needed
      if (initType->getKind() != declType->getKind()) {
        // auto casted =
        //     std::make_unique<Cast>(std::move(initializer),
        //     declType->clone());
        // casted->setExprType(declType->clone());
        // initializer = std::move(casted);
        initializer.reset(TypeChecker::convert_by_assignment(
            initializer.release(), declType));
      }
    }
    return nullptr;
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

      symTab.currentFunctionReturnType = fnType->getReturnType();

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

      symTab.currentFunctionReturnType = nullptr;
      symTab.exitScope();
    }

    return nullptr;
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