#pragma once

#include "SymbolTable.h"
#include "Type.h"

// === Forward declarations to break circular dependency ===
class Expr;
class Variable;
class ConstInt;
class ConstLong;
class Cast;
class UnaryOp;
class BinaryOp;
class Assignment;
class FuncCall;


class TypeChecker {
public:
  static Expr *convert_to(Expr *e, const Type *target);
  static const Type *get_common_type(const Type *t1, const Type *t2);
  static int getTypeSize(const Type *t);
  static const Type *get_common_pointer_type(Expr *e1, Expr *e2);
  static bool is_null_pointer_constant(Expr *e);
  static Expr *convert_by_assignment(Expr *e, const Type *target_type);
  static bool is_arithmetic_type(const Type *t); 
};
