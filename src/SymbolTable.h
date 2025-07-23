#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include <iostream>

enum class SymbolType { VARIABLE, FUNCTION };

struct SymbolInfo {
    SymbolType type;
    std::vector<std::string> params; // Used only for functions
    bool isDefined = false; // Used to check if a function is defined
};

class SymbolTable {
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes;
    std::unordered_map<std::string, SymbolInfo> globalScope; // Stores function definitions

public:
    void enterScope() { scopes.emplace_back(); }

    void exitScope() { 
        if (!scopes.empty()) {
            scopes.pop_back(); 
        } else {
            std::cerr << "ERROR: No scope to exit!" << std::endl;
        }
    }

    // Declare a variable or function
    bool declareVariable(const std::string &name) {
        if (scopes.empty()) enterScope(); // Ensure there is at least one scope

        if (scopes.back().count(name)) return false; // Variable already declared in this scope
        scopes.back()[name] = {SymbolType::VARIABLE, {}};
        return true;
    }

    // bool declareFunction(const std::string &name, const std::vector<std::string> &params) {
    //     if (globalScope.count(name)) return false; // Function already declared
    //     globalScope[name] = {SymbolType::FUNCTION, params};
    //     return true;
    // }

    bool declareFunction(const std::string &name, const std::vector<std::string> &params, bool hasBody) {
        auto it = globalScope.find(name);

        if (it != globalScope.end()) {
            SymbolInfo &info = it->second;

            if (info.type != SymbolType::FUNCTION) return false;

            if (info.params.size() != params.size()) {
                std::cerr << "ERROR: Conflicting declaration of function '" << name << "'" << std::endl;
                return false;
            }

            if (hasBody) {
                if (info.isDefined) {
                    std::cerr << "ERROR: Multiple definitions of function '" << name << "'" << std::endl;
                    return false;
                }
                info.isDefined = true; // Mark as now defined
            }

            return true; // Redeclaration allowed (matching prototype)
        }

        // First time seeing this function
        globalScope[name] = {SymbolType::FUNCTION, params, hasBody};
        return true;
    }


    // Retrieve function parameters
    std::optional<std::vector<std::string>> getFunctionParams(const std::string &name) {
        if (globalScope.count(name) && globalScope[name].type == SymbolType::FUNCTION) {
            return globalScope[name].params;
        }
        return std::nullopt; // Function not found
    }

    // Resolve a symbol (variable or function)
    std::optional<SymbolInfo> resolve(const std::string &name) {
        // Check local scopes for variables
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->count(name)) return it->at(name);
        }

        // Check global scope for functions
        if (globalScope.count(name)) return globalScope.at(name);

        return std::nullopt; // Not found
    }

    // Check if a function exists
    bool isFunction(const std::string &name) {
        return globalScope.count(name) && globalScope[name].type == SymbolType::FUNCTION;
    }

    // Check if a variable exists
    bool isVariable(const std::string &name) {
        return resolve(name).has_value() && resolve(name)->type == SymbolType::VARIABLE;
    }
};
