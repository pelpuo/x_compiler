#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Type.h"
#include "lexer.h"

enum class SymbolType { VARIABLE, FUNCTION };

// New: kind of initializer for static/file-scope variables
enum class InitKind { NoInitializer, Tentative, Initial };

struct StaticInit {
  enum class Kind { IntInit, LongInit } kind;
  union {
    int intVal;
    long long longVal;
  };

  StaticInit(int val) : kind(Kind::IntInit), intVal(val) {}
  StaticInit(long long val) : kind(Kind::LongInit), longVal(val) {}

  StaticInit(const StaticInit &other) {
    kind = other.kind;
    if (kind == Kind::IntInit)
      intVal = other.intVal;
    else
      longVal = other.longVal;
  }

  StaticInit &operator=(const StaticInit &other) {
    kind = other.kind;
    if (kind == Kind::IntInit)
      intVal = other.intVal;
    else
      longVal = other.longVal;
    return *this;
  }
};

struct InitialValue {
  InitKind kind = InitKind::NoInitializer;
  std::optional<StaticInit> value =
      std::nullopt; // Only set when kind == Initial
};

struct SymbolInfo {
  SymbolType type;
  std::vector<std::string> params; // Used only for functions

  bool isDefined = false; // For functions
  StorageClass storageClass = StorageClass::NONE;
  bool hasLinkage = false; // External or static symbols
  bool isGlobal = false;   // File scope or static local

  // For variables
  InitialValue initValue;
  std::unique_ptr<Type> declaredType; // stores variable or function return type

  // Custom copy constructor
  SymbolInfo(const SymbolInfo &other)
      : type(other.type), params(other.params), isDefined(other.isDefined),
        storageClass(other.storageClass), hasLinkage(other.hasLinkage),
        isGlobal(other.isGlobal), initValue(other.initValue),
        declaredType(other.declaredType ? other.declaredType->clone()
                                        : nullptr) {}

  // Custom copy assignment operator
  SymbolInfo &operator=(const SymbolInfo &other) {
    if (this != &other) {
      type = other.type;
      params = other.params;
      isDefined = other.isDefined;
      storageClass = other.storageClass;
      hasLinkage = other.hasLinkage;
      isGlobal = other.isGlobal;
      initValue = other.initValue;
      declaredType = other.declaredType ? other.declaredType->clone() : nullptr;
    }
    return *this;
  }

  // Default constructor
  SymbolInfo() = default;
};

class SymbolTable {

  std::vector<std::unordered_map<std::string, SymbolInfo>> scopes;

public:
  std::unordered_map<std::string, SymbolInfo>
      globalScope; // Stores file-scope decls and all functions
  void enterScope() { scopes.emplace_back(); }

  bool inGlobalScope() const {
    return scopes.empty(); // No local scope means file-scope
  }

  void exitScope() {
    if (!scopes.empty()) {
      scopes.pop_back();
    } else {
      std::cerr << "ERROR: No scope to exit!" << std::endl;
    }
  }

  // Declare a variable with storage class and initializer info
  bool declareFileScopeVariable(const std::string &name,
                                std::unique_ptr<Type> type, StorageClass sc,
                                InitialValue newInit) {
    bool isGlobal = (sc != StorageClass::STATIC);

    auto it = globalScope.find(name);
    if (it != globalScope.end()) {
      SymbolInfo &old = it->second;

      if (old.type == SymbolType::FUNCTION) {
        std::cerr << "ERROR: Function redeclared as variable: " << name << "\n";
        return false;
      }

      if (sc == StorageClass::EXTERN) {
        isGlobal = old.isGlobal;
      } else if (old.isGlobal != isGlobal) {
        std::cerr << "ERROR: Conflicting variable linkage for " << name << "\n";
        return false;
      }

      if (hasInitializer(old) && newInit.kind == InitKind::Initial) {
        std::cerr << "ERROR: Conflicting file-scope variable definitions: "
                  << name << "\n";
        return false;
      }

      if (hasInitializer(old)) {
        newInit = old.initValue;
      } else if (newInit.kind != InitKind::Initial &&
                 old.initValue.kind == InitKind::Tentative) {
        newInit.kind = InitKind::Tentative;
      }

      // ⬅ Set declared type only if missing
      if (!old.declaredType && type) {
        old.declaredType = std::move(type);
      }

      old.storageClass = sc;
      old.isGlobal = isGlobal;
      old.hasLinkage = true;
      old.initValue = newInit;
      return true;
    }

    // New declaration
    SymbolInfo info;
    info.type = SymbolType::VARIABLE;
    info.storageClass = sc;
    info.isGlobal = isGlobal;
    info.hasLinkage = true;
    info.initValue = newInit;
    info.declaredType = std::move(type);

    globalScope[name] = std::move(info);
    return true;
  }

  bool declareBlockScopeVariable(const std::string &name,
                                 std::unique_ptr<Type> type, StorageClass sc,
                                 const std::optional<int> &constantInit) {
    SymbolInfo info;
    info.type = SymbolType::VARIABLE;
    info.storageClass = sc;
    info.hasLinkage = (sc == StorageClass::EXTERN);
    info.isGlobal = false;
    info.declaredType = std::move(type);

    if (sc == StorageClass::EXTERN) {
      if (constantInit.has_value()) {
        std::cerr
            << "ERROR: Initializer on local extern variable declaration\n";
        return false;
      }

      auto prev = resolve(name);
      if (prev && prev->type == SymbolType::FUNCTION) {
        std::cerr << "ERROR: Function redeclared as variable '" << name
                  << "'\n";
        return false;
      }

      if (!prev) {
        InitialValue noInit;
        noInit.kind = InitKind::NoInitializer;
        return declareVariable(name, info.declaredType->clone(), sc, noInit);
      }
      return true;
    } else if (sc == StorageClass::STATIC) {
      InitialValue init;
      init.kind = InitKind::Initial;
      // init.value = constantInit.value_or(0);
      if (constantInit.has_value()) {
        if (info.declaredType.get()->getKind() == Type::Kind::LONG) {
          init.value = StaticInit(static_cast<long long>(constantInit.value()));
        } else {
          init.value = StaticInit(constantInit.value());
        }
      }

      return declareVariable(name, info.declaredType->clone(), sc, init);
    } else {
      InitialValue init;
      init.kind = InitKind::NoInitializer;
      return declareVariable(name, info.declaredType->clone(), sc, init);
    }
  }

  bool declareVariable(const std::string &name, std::unique_ptr<Type> type,
                       StorageClass sc = StorageClass::NONE,
                       InitialValue init = {}) {
    SymbolInfo info;
    info.type = SymbolType::VARIABLE;
    info.storageClass = sc;
    info.initValue = init;
    info.hasLinkage = inGlobalScope() || sc == StorageClass::EXTERN;
    info.isGlobal = inGlobalScope() || sc == StorageClass::STATIC;
    info.declaredType = std::move(type);

    if (inGlobalScope()) {
      if (globalScope.count(name))
        return false;
      globalScope[name] = std::move(info);
    } else {
      if (scopes.empty())
        enterScope();
      if (scopes.back().count(name))
        return false;
      scopes.back()[name] = std::move(info);
    }

    return true;
  }

  // Declare a function (same as before)
  bool declareFunction(const std::string &name,
                       const std::vector<std::string> &params,
                       std::unique_ptr<Type> returnType, bool hasBody,
                       StorageClass sc = StorageClass::NONE) {
    auto it = globalScope.find(name);
    bool isGlobal = (sc != StorageClass::STATIC);

    if (it != globalScope.end()) {
      SymbolInfo &info = it->second;

      if (info.type != SymbolType::FUNCTION)
        return false;

      if (info.params.size() != params.size()) {
        std::cerr << "ERROR: Conflicting declaration of function '" << name
                  << "'\n";
        return false;
      }

      if (hasBody && info.isDefined) {
        std::cerr << "ERROR: Multiple definitions of function '" << name
                  << "'\n";
        return false;
      }

      if (info.isGlobal && sc == StorageClass::STATIC) {
        std::cerr
            << "ERROR: Static function declaration follows non-static for '"
            << name << "'\n";
        return false;
      }

      if (!info.declaredType && returnType) {
        info.declaredType = std::move(returnType);
      }

      info.isDefined |= hasBody;
      info.storageClass = sc;
      info.isGlobal = isGlobal;
      return true;
    }

    SymbolInfo newInfo;
    newInfo.type = SymbolType::FUNCTION;
    newInfo.params = params;
    newInfo.isDefined = hasBody;
    newInfo.storageClass = sc;
    newInfo.hasLinkage = true;
    newInfo.isGlobal = isGlobal;
    newInfo.declaredType = std::move(returnType);

    globalScope[name] = std::move(newInfo);
    return true;
  }

  std::optional<std::vector<std::string>>
  getFunctionParams(const std::string &name) {
    if (globalScope.count(name) &&
        globalScope[name].type == SymbolType::FUNCTION) {
      return globalScope[name].params;
    }
    return std::nullopt;
  }

  std::optional<SymbolInfo> resolve(const std::string &name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
      if (it->count(name))
        return it->at(name);
    }
    if (globalScope.count(name))
      return globalScope.at(name);
    return std::nullopt;
  }

  bool isFunction(const std::string &name) {
    return globalScope.count(name) &&
           globalScope[name].type == SymbolType::FUNCTION;
  }

  bool isVariable(const std::string &name) {
    return resolve(name).has_value() &&
           resolve(name)->type == SymbolType::VARIABLE;
  }

  // Helper to check if variable has an initializer
  static bool hasInitializer(const SymbolInfo &info) {
    return info.initValue.kind == InitKind::Initial;
  }
};
