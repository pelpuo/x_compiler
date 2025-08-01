// Type.h
#pragma once
#include <memory>
#include <vector>

class Type {
public:
  enum class Kind { INT, LONG, FUNCTION, UNSIGNED_INT, UNSIGNED_LONG, DOUBLE };
  virtual Kind getKind() const = 0;
  virtual ~Type() = default;

  virtual std::unique_ptr<Type> clone() const = 0;
  virtual std::string toString() const = 0;
};

class IntType : public Type {
public:
  Kind getKind() const override { return Kind::INT; }
  std::string toString() const override { return "int"; }
  std::unique_ptr<Type> clone() const override {
    return std::make_unique<IntType>(*this);
  }
};

class LongType : public Type {
public:
  Kind getKind() const override { return Kind::LONG; }
  std::string toString() const override { return "long"; }
  std::unique_ptr<Type> clone() const override {
    return std::make_unique<LongType>(*this);
  }
};

class UnsignedIntType : public Type {
public:
  Kind getKind() const override { return Kind::UNSIGNED_INT; }
  std::string toString() const override { return "unsigned_int"; }
  std::unique_ptr<Type> clone() const override {
    return std::make_unique<UnsignedIntType>(*this);
  }
};

class UnsignedLongType : public Type {
public:
  Kind getKind() const override { return Kind::UNSIGNED_LONG; }
  std::string toString() const override { return "unsigned_long"; }
  std::unique_ptr<Type> clone() const override {
    return std::make_unique<UnsignedLongType>(*this);
  }
};

class DoubleType : public Type {
public:
  Kind getKind() const override { return Kind::DOUBLE; }
  std::string toString() const override { return "double"; }
  std::unique_ptr<Type> clone() const override {
    return std::make_unique<DoubleType>(*this);
  }
};

class FunctionType : public Type {
  std::vector<std::unique_ptr<Type>> paramTypes;
  std::unique_ptr<Type> returnType;

public:
  FunctionType(std::vector<std::unique_ptr<Type>> params,
               std::unique_ptr<Type> ret)
      : paramTypes(std::move(params)), returnType(std::move(ret)) {}

  // Deep copy constructor
  FunctionType(const FunctionType &other) {
    for (const auto &param : other.paramTypes) {
      if (param)
        paramTypes.push_back(param->clone());
      else
        paramTypes.push_back(nullptr);
    }
    returnType = other.returnType ? other.returnType->clone() : nullptr;
  }

  Kind getKind() const override { return Kind::FUNCTION; }

  std::string toString() const override {
    std::string result = "function(";
    for (const auto &param : paramTypes) {
      result += param->toString() + ", ";
    }
    if (!paramTypes.empty()) {
      result.pop_back(); // Remove space
      result.pop_back(); // Remove comma
    }
    result += ") -> " + returnType->toString();
    return result;
  }

  const std::vector<std::unique_ptr<Type>> &getParamTypes() const {
    return paramTypes;
  }

  const Type *getReturnType() const { return returnType.get(); }

  std::unique_ptr<Type> clone() const override {
    return std::make_unique<FunctionType>(*this); 
  }
};
