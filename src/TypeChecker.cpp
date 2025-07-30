#include "AST.h"
#include "TypeChecker.h"

Expr *TypeChecker::typecheck(Expr *e, SymbolTable &symbols) {
  if (auto *v = dynamic_cast<Variable *>(e)) {
    return typecheck_var(v, symbols);
  } else if (auto *c = dynamic_cast<ConstInt *>(e)) {
    return typecheck_constint(c);
  } else if (auto *c = dynamic_cast<ConstLong *>(e)) {
    return typecheck_constlong(c);
  } else if (auto *c = dynamic_cast<Cast *>(e)) {
    return typecheck_cast(c, symbols);
  } else if (auto *u = dynamic_cast<UnaryOp *>(e)) {
    return typecheck_unary(u, symbols);
  } else if (auto *b = dynamic_cast<BinaryOp *>(e)) {
    return typecheck_binary(b, symbols);
  } else if (auto *a = dynamic_cast<Assignment *>(e)) {
    return typecheck_assignment(a, symbols);
  } else if (auto *call = dynamic_cast<FuncCall *>(e)) {
    return typecheck_funccall(call, symbols);
  } else {
    std::cerr << "ERROR: TypeChecker does not support this expression type.\n";
    exit(1);
  }
}

Expr *TypeChecker::typecheck_var(Variable *v, SymbolTable &symbols) {
  auto infoOpt = symbols.resolve(v->name);
  if (!infoOpt) {
    std::cerr << "ERROR: Undeclared variable '" << v->name << "'\n";
    exit(1);
  }

  const SymbolInfo &info = *infoOpt;
  if (info.type == SymbolType::FUNCTION) {
    std::cerr << "ERROR: Function name used as variable: " << v->name << "\n";
    exit(1);
  }

  v->setExprType(info.declaredType->clone());
  return v;
}

Expr *TypeChecker::typecheck_constint(ConstInt *c) {
  c->setExprType(std::make_unique<IntType>());
  return c;
}

Expr *TypeChecker::typecheck_constlong(ConstLong *c) {
  c->setExprType(std::make_unique<LongType>());
  return c;
}

Expr *TypeChecker::typecheck_cast(Cast *c, SymbolTable &symbols) {
  Expr *inner = typecheck(c->expr.get(), symbols);
  c->expr.reset(inner);
  c->setExprType(c->type->clone());
  return c;
}

Expr *TypeChecker::typecheck_unary(UnaryOp *u, SymbolTable &symbols) {
  Expr *inner = typecheck(u->operand.get(), symbols);
  u->operand.reset(inner);
  if (u->op == TokenType::LOGICAL_NOT) {
    u->setExprType(std::make_unique<IntType>());
  } else {
    u->setExprType(inner->getExprType()->clone());
  }
  return u;
}

Expr *TypeChecker::typecheck_binary(BinaryOp *b, SymbolTable &symbols) {
  Expr *lhs = typecheck(b->left.get(), symbols);
  Expr *rhs = typecheck(b->right.get(), symbols);
  b->left.reset(lhs);
  b->right.reset(rhs);

  const Type *lt = lhs->getExprType();
  const Type *rt = rhs->getExprType();
  if (!lt || !rt) {
    std::cerr << "ERROR: Missing type info in binary operands\n";
    exit(1);
  }

  if (b->op == TokenType::LOGICAL_AND || b->op == TokenType::LOGICAL_OR) {
    b->setExprType(std::make_unique<IntType>());
    return b;
  }

  const Type *common = get_common_type(lt, rt);
  lhs = convert_to(lhs, common);
  rhs = convert_to(rhs, common);
  b->left.reset(lhs);
  b->right.reset(rhs);

  std::unique_ptr<Type> result;
  switch (b->op) {
  case TokenType::PLUS:
  case TokenType::MINUS:
  case TokenType::MUL:
  case TokenType::DIV:
  case TokenType::MOD:
    result = common->clone();
    break;
  default:
    result = std::make_unique<IntType>();
    break;
  }

  b->setExprType(std::move(result));
  return b;
}

Expr *TypeChecker::typecheck_assignment(Assignment *a, SymbolTable &symbols) {
  // Typecheck both sides
  std::unique_ptr<Expr> lhs(typecheck(a->name.release(), symbols));
  std::unique_ptr<Expr> rhs(typecheck(a->value.release(), symbols));

  const Type *lt = lhs->getExprType();
  const Type *rt = rhs->getExprType();

  if (!lt || !rt) {
    std::cerr << "ERROR: Assignment operands missing type info\n";
    exit(1);
  }

  // Convert RHS to LHS type
  rhs.reset(convert_to(rhs.release(), lt));

  // Store updated pointers
  a->name = std::move(lhs);
  a->value = std::move(rhs);

  // The type of the assignment expression is the LHS type
  a->setExprType(lt->clone());
  return a;
}

Expr *TypeChecker::typecheck_funccall(FuncCall *call, SymbolTable &symbols) {
  auto infoOpt = symbols.resolve(call->name);
  if (!infoOpt.has_value()) {
    std::cerr << "ERROR: Call to undeclared function " << call->name << "\n";
    exit(1);
  }

  const SymbolInfo &info = *infoOpt;
  if (!info.declaredType ||
      info.declaredType->getKind() != Type::Kind::FUNCTION) {
    std::cerr << "ERROR: Symbol is not a function: " << call->name << "\n";
    exit(1);
  }

  const auto *fnType =
      static_cast<const FunctionType *>(info.declaredType.get());
  const auto &paramTypes = fnType->getParamTypes();

  if (paramTypes.size() != call->args->size()) {
    std::cerr << "ERROR: Function call argument count mismatch\n";
    exit(1);
  }

  for (size_t i = 0; i < paramTypes.size(); ++i) {
    Expr *arg = typecheck(call->args->args[i].get(), symbols);
    const Type *expected = paramTypes[i].get();
    arg = convert_to(arg, expected);
    call->args->args[i].reset(arg);
  }

  call->setExprType(fnType->getReturnType()->clone());
  return call;
}

Expr *TypeChecker::convert_to(Expr *e, const Type *target) {
  if (e->getExprType()->getKind() == target->getKind())
    return e;
  auto cast = std::make_unique<Cast>(std::unique_ptr<Expr>(e), target->clone());
  cast->setExprType(target->clone());
  return cast.release();
}

const Type *TypeChecker::get_common_type(const Type *t1, const Type *t2) {
  if (t1->getKind() == t2->getKind())
    return t1;
  // return new LongType();
  static LongType longTypeSingleton;
  return &longTypeSingleton;
}
// };
