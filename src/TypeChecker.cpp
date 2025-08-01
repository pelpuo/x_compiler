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
    default:
      std::cerr << "Unknown type in getTypeSize()\n";
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

  // ❶ If types are identical
  if (t1->getKind() == t2->getKind()) {
    return t1;
  }
  
  // ❶ If types are identical
  if ((t1->getKind() == Type::Kind::DOUBLE) || (t2->getKind() == Type::Kind::DOUBLE)) {
    return std::make_unique<DoubleType>().release();
  }

  auto isSigned = [](const Type *t) -> bool {
    return t->getKind() == Type::Kind::INT || t->getKind() == Type::Kind::LONG;
  };

  int size1 = getTypeSize(t1);
  int size2 = getTypeSize(t2);

  // ❷ If sizes are equal
  if (size1 == size2) {
    if (isSigned(t1)) {
      return t2;
    } else {
      return t1;
    }
  }

  // ❸ Otherwise return the larger type
  return size1 > size2 ? t1 : t2;
}

