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
  Expr *typecheck(Expr *e, SymbolTable &symbols);
  Expr *typecheck_var(Variable *v, SymbolTable &symbols);
  Expr *typecheck_constint(ConstInt *c);
  Expr *typecheck_constlong(ConstLong *c);
  Expr *typecheck_cast(Cast *c, SymbolTable &symbols);
  Expr *typecheck_unary(UnaryOp *u, SymbolTable &symbols);
  Expr *typecheck_binary(BinaryOp *b, SymbolTable &symbols);
  Expr *typecheck_assignment(Assignment *a, SymbolTable &symbols);
  Expr *typecheck_funccall(FuncCall *call, SymbolTable &symbols);
};
