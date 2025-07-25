#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include <iostream>

#include "lexer.h"

enum class SymbolType { VARIABLE, FUNCTION };

// New: kind of initializer for static/file-scope variables
enum class InitKind { NoInitializer, Tentative, Initial };

struct InitialValue {
    InitKind kind = InitKind::NoInitializer;
    std::optional<int> value = std::nullopt; // Only set when kind == Initial
};

struct SymbolInfo {
    SymbolType type;
    std::vector<std::string> params; // Used only for functions

    bool isDefined = false;          // For functions
    StorageClass storageClass = StorageClass::NONE;
    bool hasLinkage = false;         // External or static symbols
    bool isGlobal = false;           // File scope or static local

    // For variables
    InitialValue initValue;
};

class SymbolTable {
    
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes;
public:
    std::unordered_map<std::string, SymbolInfo> globalScope; // Stores file-scope decls and all functions
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
                              StorageClass sc,
                              InitialValue newInit) {
        bool isGlobal = (sc != StorageClass::STATIC);

        auto it = globalScope.find(name);
        if (it != globalScope.end()) {
            SymbolInfo &old = it->second;

            if (old.type == SymbolType::FUNCTION) {
                std::cerr << "ERROR: Function redeclared as variable: " << name << "\n";
                return false;
            }

            // Reconcile linkage
            if (sc == StorageClass::EXTERN) {
                isGlobal = old.isGlobal;
            } else if (old.isGlobal != isGlobal) {
                std::cerr << "ERROR: Conflicting variable linkage for " << name << "\n";
                return false;
            }

            // Reconcile initializer
            if (hasInitializer(old) && newInit.kind == InitKind::Initial) {
                std::cerr << "ERROR: Conflicting file-scope variable definitions: " << name << "\n";
                return false;
            }

            // Merge initializers
            if (hasInitializer(old)) {
                newInit = old.initValue;
            } else if (newInit.kind != InitKind::Initial && old.initValue.kind == InitKind::Tentative) {
                newInit.kind = InitKind::Tentative;
            }

            // Update entry
            old.storageClass = sc;
            old.isGlobal = isGlobal;
            old.hasLinkage = true;
            old.initValue = newInit;
            return true;
        }

        // First time seen
        SymbolInfo info;
        info.type = SymbolType::VARIABLE;
        info.storageClass = sc;
        info.isGlobal = isGlobal;
        info.hasLinkage = true;
        info.initValue = newInit;

        globalScope[name] = info;
        return true;
    }

    bool declareBlockScopeVariable(const std::string &name,
                                    StorageClass sc,
                                    const std::optional<int> &constantInit) {
        SymbolInfo info;
        info.type = SymbolType::VARIABLE;
        info.storageClass = sc;
        info.hasLinkage = (sc == StorageClass::EXTERN);
        // info.isGlobal = (sc == StorageClass::EXTERN); // extern locals are globally visible
        info.isGlobal = false;  // ← always false for locals

        if (sc == StorageClass::EXTERN) {
            if (constantInit.has_value()) {
                std::cerr << "ERROR: Initializer on local extern variable declaration\n";
                return false;
            }

            auto prev = resolve(name);
            if (prev && prev->type == SymbolType::FUNCTION) {
                std::cerr << "ERROR: Function redeclared as variable '" << name << "'\n";
                return false;
            }

            if (!prev) {
                info.initValue.kind = InitKind::NoInitializer;
                return declareVariable(name, sc, info.initValue);
            }
            return true; // Valid extern redeclaration

        } else if (sc == StorageClass::STATIC) {
            // if (constantInit.has_value()) {
            //     info.initValue.kind = InitKind::Initial;
            //     info.initValue.value = constantInit.value();
            // } else {
            //     info.initValue.kind = InitKind::Initial;
            //     info.initValue.value = 0; // Default init to zero
            // }

            // info.isGlobal = false;
            // return declareVariable(name, sc, info.initValue);

            info.initValue.kind = InitKind::Initial;
            info.initValue.value = constantInit.value_or(0);

            // 🔥 Push into globalScope instead of local scopes
            if (globalScope.count(name)) {
                std::cerr << "ERROR: Duplicate static local variable: " << name << "\n";
                return false;
            }

            globalScope[name] = info;
            return true;

        } else {
            // Automatic variable
            info.initValue.kind = InitKind::NoInitializer;
            info.isGlobal = false;
            return declareVariable(name, sc, info.initValue);
        }
    }

    bool declareVariable(const std::string &name,
                         StorageClass sc = StorageClass::NONE,
                         InitialValue init = {}) {
        SymbolInfo info;
        info.type = SymbolType::VARIABLE;
        info.storageClass = sc;
        info.initValue = init;
        info.hasLinkage = inGlobalScope() || sc == StorageClass::EXTERN;
        info.isGlobal = inGlobalScope() || sc == StorageClass::STATIC;

        if (inGlobalScope()) {
            if (globalScope.count(name)) return false;
            globalScope[name] = info;
        } else {
            if (scopes.empty()) enterScope();
            if (scopes.back().count(name)) return false;

            scopes.back()[name] = info;
        }

        return true;
    }

    // Declare a function (same as before)
    bool declareFunction(const std::string &name,
                        const std::vector<std::string> &params,
                        bool hasBody,
                        StorageClass sc = StorageClass::NONE) {
        auto it = globalScope.find(name);
        bool isGlobal = (sc != StorageClass::STATIC);

        if (it != globalScope.end()) {
            SymbolInfo &info = it->second;

            if (info.type != SymbolType::FUNCTION) return false;

            // Check for prototype mismatch
            if (info.params.size() != params.size()) {
                std::cerr << "ERROR: Conflicting declaration of function '" << name << "'" << std::endl;
                return false;
            }

            // Check for multiple definitions
            if (hasBody && info.isDefined) {
                std::cerr << "ERROR: Multiple definitions of function '" << name << "'" << std::endl;
                return false;
            }

            // Check for static after extern
            if (info.isGlobal && sc == StorageClass::STATIC) {
                std::cerr << "ERROR: Static function declaration follows non-static for '" << name << "'\n";
                return false;
            }

            // Retain original visibility
            isGlobal = info.isGlobal;

            // Update definition status
            info.isDefined |= hasBody;
            info.storageClass = sc;
            info.isGlobal = isGlobal;
            return true;
        }

        // New declaration
        SymbolInfo newInfo;
        newInfo.type = SymbolType::FUNCTION;
        newInfo.params = params;
        newInfo.isDefined = hasBody;
        newInfo.storageClass = sc;
        newInfo.hasLinkage = true;
        newInfo.isGlobal = isGlobal;

        globalScope[name] = newInfo;
        return true;
    }


    std::optional<std::vector<std::string>> getFunctionParams(const std::string &name) {
        if (globalScope.count(name) && globalScope[name].type == SymbolType::FUNCTION) {
            return globalScope[name].params;
        }
        return std::nullopt;
    }

    std::optional<SymbolInfo> resolve(const std::string &name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->count(name)) return it->at(name);
        }
        if (globalScope.count(name)) return globalScope.at(name);
        return std::nullopt;
    }

    bool isFunction(const std::string &name) {
        return globalScope.count(name) && globalScope[name].type == SymbolType::FUNCTION;
    }

    bool isVariable(const std::string &name) {
        return resolve(name).has_value() && resolve(name)->type == SymbolType::VARIABLE;
    }

    // Helper to check if variable has an initializer
    static bool hasInitializer(const SymbolInfo &info) {
        return info.initValue.kind == InitKind::Initial;
    }
};
