#include "TypeChecker.h"
#include "AST.h"

int TypeChecker::getTypeSize(const Type *t) {
  switch (t->getKind()) {
  case Type::Kind::INT:
  case Type::Kind::UNSIGNED_INT:
    return 4;
  case Type::Kind::LONG:
  case Type::Kind::UNSIGNED_LONG:
    return 8;
  case Type::Kind::POINTER:
    return 8; // Assuming 64-bit pointers
  default:
    std::cerr << "Unknown type in getTypeSize(): " << t->toString() << "\n";
    exit(1);
  }
}

Expr *TypeChecker::convert_to(Expr *e, const Type *target) {
  // cout << "Converting " << e->getExprType()->toString()
  //     << " to " << target->toString() << "for expression: ";
  //     e->print();

  if (e->getExprType()->getKind() == target->getKind())
    return e;
  auto cast = std::make_unique<Cast>(std::unique_ptr<Expr>(e), target->clone());
  cast->setExprType(target->clone());
  return cast.release();
}

const Type *TypeChecker::get_common_type(const Type *t1, const Type *t2) {
  if (!t1 || !t2) {
    std::cerr << "ERROR: get_common_type received null type\n";
    exit(1);
  }

  // Disallow pointer types here — use get_common_pointer_type instead
  if (t1->isPointerType() || t2->isPointerType()) {
    std::cerr << "ERROR: get_common_type called with pointer type\n";
    exit(1);
  }

  // Same type
  if (t1->getKind() == t2->getKind()) {
    return t1;
  }

  // Double promotion
  if (t1->isDouble() || t2->isDouble()) {
    return new DoubleType();
  }

  auto isSigned = [](const Type *t) -> bool {
    return t->getKind() == Type::Kind::INT || t->getKind() == Type::Kind::LONG;
  };

  int size1 = getTypeSize(t1);
  int size2 = getTypeSize(t2);

  if (size1 == size2) {
    return isSigned(t1) ? t2 : t1;
  }

  return size1 > size2 ? t1 : t2;
}

bool TypeChecker::is_null_pointer_constant(Expr *e) {
  if (auto ci = dynamic_cast<ConstInt *>(e)) {
    return ci->value == 0;
  }
  if (auto cul = dynamic_cast<ConstLong *>(e)) {
    return cul->value == 0;
  }
  // If you also have ConstUInt, ConstULong:
  if (auto cui = dynamic_cast<ConstUnsignedInt *>(e)) {
    return cui->value == 0;
  }
  if (auto culng = dynamic_cast<ConstUnsignedLong *>(e)) {
    return culng->value == 0;
  }
  return false;
}

const Type *TypeChecker::get_common_pointer_type(Expr *e1, Expr *e2) {
  const Type *t1 = e1->getExprType();
  const Type *t2 = e2->getExprType();

  if (t1->equals(t2)) {
    return t1;
  } else if (is_null_pointer_constant(e1)) {
    return t2;
  } else if (is_null_pointer_constant(e2)) {
    return t1;
  } else {
    std::cerr << "Incompatible pointer types: " << t1->toString() << " vs "
              << t2->toString() << "\n";
    exit(1);
  }
}

Expr *TypeChecker::convert_by_assignment(Expr *e, const Type *target_type) {
  const Type *src_type = e->getExprType();
  if (!src_type || !target_type) {
    std::cerr << "ERROR: Null type passed to convert_by_assignment\n";
    exit(1);
  }

  // 1. Exact type match: no conversion needed
  if (src_type->equals(target_type)) {
    return e;
  }

  // 2. Both arithmetic types? (int, unsigned int, long, unsigned long, double)
  bool src_arith = is_arithmetic_type(src_type);
  bool target_arith = is_arithmetic_type(target_type);
  if (src_arith && target_arith) {
    return convert_to(e, target_type);
  }

  // 3. Null pointer constant to pointer type
  if (is_null_pointer_constant(e) && target_type->isPointerType()) {
    return convert_to(e, target_type);
  }

  // 4. Otherwise, incompatible assignment
  std::cerr << "ERROR: Cannot convert expression of type " << src_type->toString()
            << " to " << target_type->toString() << " in assignment\n";
  exit(1);
}

// Helper: Checks if type is arithmetic
bool TypeChecker::is_arithmetic_type(const Type *t) {
  if (!t) return false;
  switch (t->getKind()) {
    case Type::Kind::INT:
    case Type::Kind::UNSIGNED_INT:
    case Type::Kind::LONG:
    case Type::Kind::UNSIGNED_LONG:
    case Type::Kind::DOUBLE:
      return true;
    default:
      return false;
  }
}
