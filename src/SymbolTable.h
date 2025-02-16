#include <unordered_map>
#include <string>
#include <vector>

class SymbolTable {
    std::vector<std::unordered_map<std::string, bool>> scopes;

public:
    void enterScope() { scopes.emplace_back(); }

    void exitScope() { scopes.pop_back(); }

    bool declare(const std::string &name) {
        if (scopes.back().count(name)) return false; // Duplicate variable in the same scope
        scopes.back()[name] = true;
        return true;
    }

    bool resolve(const std::string &name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->count(name)) return true; // Variable found in an outer scope
        }
        return false; // Undeclared variable
    }
};
