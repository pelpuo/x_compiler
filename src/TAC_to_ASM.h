#pragma once

#include "TAC.h"
#include <fstream>
#include <map>

class TACtoASM {
private:
  std::ofstream &outfile;
  int tempVarCounter = 0; // Track temporary registers (t0, t1, ...)
  int stackOffset = 0;    // Track stack memory offset
  int stackSize = 64;     // Stack size for function prologue/epilogue
  static int paramIndex;  // Track param position

  // std::map<std::string, int> varMap; // Maps variables to stack offsets
  struct VarInfo {
    int offset;
    std::string type; // "int" or "long"
  };
  std::map<std::string, VarInfo> varMap;

  std::map<std::string, std::string>
      registerMap; // Maps temp vars to RISC-V registers
  std::vector<std::string> pendingArgs;

  std::string getTempReg() {
    return "t" + std::to_string(tempVarCounter++ % 7); // Reuse t0-t6
  }

  std::string mapToRegister(const std::string &tempVar) {
    if (registerMap.find(tempVar) == registerMap.end()) {
      registerMap[tempVar] = getTempReg();
    }
    return registerMap[tempVar];
  }

  std::string mapToArgRegister(const std::string &tempVar, int index) {
    std::string argReg = "a" + std::to_string(index);
    registerMap[tempVar] = argReg; // Map tempVar to a0, a1, ...
    return argReg;
  }

  void emitGlobalData(const std::vector<TAC> &dataTACs) {
    if (dataTACs.empty())
      return;
    outfile << ".data\n";
    for (const auto &tac : dataTACs) {
      const std::string &name = tac.arg1;
      bool isGlobal = true;
      std::string type = tac.arg2;
      if (tac.result == "-1") {
        outfile << ".extern " << name << "\n";
      } else {
        if (isGlobal) {
          outfile << ".globl " << name << "\n";
        }
        outfile << name << ":\n";
        if (type == "int") {
          outfile << "    .word " << tac.result << "\n";
        } else {
          outfile << "    .quad " << tac.result << "\n";
        }

        // outfile << "    " << dir << " " << tac.result << "\n";
      }
    }
  }

  void emitPrologue(int stackSize = 64) {
    outfile << "    addi sp, sp, -" << stackSize << "\n";
    outfile << "    sd ra, " << (stackSize - 8) << "(sp)\n";
    outfile << "    sd s0, " << (stackSize - 16) << "(sp)\n";
    outfile << "    addi s0, sp, " << stackSize << "\n";
    stackOffset = -16; // Reset stack allocation
  }

  void emitEpilogue() {
    outfile << "    ld ra, " << (stackSize - 8) << "(sp)\n";
    outfile << "    ld s0, " << (stackSize - 16) << "(sp)\n";
    outfile << "    addi sp, sp, " << stackSize << "\n";
    outfile << "    ret\n";
  }

  bool isVariable(const std::string &name) {
    return !(name.size() >= 2 && name[0] == 't' && isdigit(name[1]));
  }

  std::string ensureLoaded(const std::string &name) {
    std::string reg = mapToRegister(name);
    if (isVariable(name) && varMap.find(name) != varMap.end()) {
      std::string op = (varMap[name].type == "int") ? "lw" : "ld";
      outfile << "    " << op << " " << reg << ", " << varMap[name].offset
              << "(s0)\n";
    }
    return reg;
  }

  bool isImmediate(const std::string &s) {
    if (s.empty())
      return false;
    if (isdigit(s[0]) || (s[0] == '-' && isdigit(s[1])))
      return true;
    return false;
  }

public:
  TACtoASM(std::ofstream &file) : outfile(file) {}

  void generateAssembly(const std::vector<TAC> &tacCode) {
    std::vector<TAC> dataTACs;
    std::vector<TAC> codeTACs;

    // Separate static variables from function code
    for (const auto &tac : tacCode) {
      if (tac.op == "StaticVariable") {
        dataTACs.push_back(tac);
      } else {
        codeTACs.push_back(tac);
      }
    }

    // Emit .data / .bss / .extern
    emitGlobalData(dataTACs);

    outfile << ".text\n";
    outfile << ".globl main\n";
    outfile << ".type main, @function\n";

    // Iterate over the TAC code
    for (const auto &tac : tacCode) {
      if (tac.op == "function") {
        // Each function call starts with its own stack and register space
        outfile << tac.arg1 << ":\n"; // Function label
        tempVarCounter = 0;           // Reset temp registers for each function
        varMap.clear();      // Clear the variable map for new function scope
        registerMap.clear(); // Clear the register map for new function scope

        emitPrologue(); // Emit prologue for each function
      } else if (tac.op == "RETURN") {
        // Handle return with a specific epilogue
        outfile << "    mv a0, " << ensureLoaded(tac.arg1) << "\n";
        emitEpilogue();
      }

      else if (tac.op == "store") {
        std::string srcReg = ensureLoaded(tac.arg1);
        std::string type = tac.arg2.empty() ? "long" : tac.arg2;
        std::string op = (type == "int") ? "sw" : "sd";

        // Allocate stack slot for local variable if not already done
        if (varMap.count(tac.result) == 0) {
          int size = (type == "int") ? 4 : 8;
          stackOffset -= size;
          varMap[tac.result] = {stackOffset, type};
        }

        // Emit correct store based on varMap info
        if (varMap.count(tac.result)) {
          // Local variable (on stack)
          outfile << "    " << op << " " << srcReg << ", "
                  << varMap[tac.result].offset << "(s0)\n";
        } else {
          // Global/static fallback (shouldn't happen anymore for locals)
          outfile << "    la t6, " << tac.result << "\n";
          outfile << "    " << op << " " << srcReg << ", 0(t6)\n";
        }
      }

      else if (tac.op == "load") {
        std::string dstReg = mapToRegister(tac.result);
        std::string op = (tac.arg2 == "int") ? "lw" : "ld";

        if (varMap.count(tac.arg1)) {
          // Local variable (on stack)
          outfile << "    " << op << " " << dstReg << ", "
                  << varMap[tac.arg1].offset << "(s0)\n";
        } else {
          // Global/static variable
          outfile << "    la t6, " << tac.arg1 << "\n";
          outfile << "    " << op << " " << dstReg << ", 0(t6)\n";
        }
      }

      else if (tac.op == "li") {
        // Load immediate
        outfile << "    li " << mapToRegister(tac.result) << ", " << tac.arg1
                << "\n";
      } else if (tac.op == "+") {
        std::string reg1 = ensureLoaded(tac.arg1);
        std::string dst = mapToRegister(tac.result);

        if (isImmediate(tac.arg2)) {
          outfile << "    addi " << dst << ", " << reg1 << ", " << tac.arg2
                  << "\n";
        } else {
          std::string reg2 = ensureLoaded(tac.arg2);
          outfile << "    add " << dst << ", " << reg1 << ", " << reg2 << "\n";
        }
      } else if (tac.op == "-") {
        std::string reg1 = ensureLoaded(tac.arg1);
        std::string dst = mapToRegister(tac.result);

        if (isImmediate(tac.arg2)) {
          // Emit: dst = reg1 + (-imm)
          int imm = std::stoi(tac.arg2);
          outfile << "    addi " << dst << ", " << reg1 << ", " << -imm << "\n";
        } else {
          std::string reg2 = ensureLoaded(tac.arg2);
          outfile << "    sub " << dst << ", " << reg1 << ", " << reg2 << "\n";
        }
      }

      else if (tac.op == "*") {
        // Multiplication
        std::string reg1 = ensureLoaded(tac.arg1);
        std::string reg2 = ensureLoaded(tac.arg2);
        std::string dst = mapToRegister(tac.result);
        outfile << "    mul " << dst << ", " << reg1 << ", " << reg2 << "\n";
      } else if (tac.op == "/") {
        // Division
        std::string reg1 = ensureLoaded(tac.arg1);
        std::string reg2 = ensureLoaded(tac.arg2);
        std::string dst = mapToRegister(tac.result);
        outfile << "    div " << dst << ", " << reg1 << ", " << reg2 << "\n";
      } else if (tac.op == "%") {
        // Division
        std::string reg1 = ensureLoaded(tac.arg1);
        std::string reg2 = ensureLoaded(tac.arg2);
        std::string dst = mapToRegister(tac.result);
        outfile << "    rem " << dst << ", " << reg1 << ", " << reg2 << "\n";
      } else if (tac.op == "&") {
        // Division
        std::string reg1 = ensureLoaded(tac.arg1);
        std::string reg2 = ensureLoaded(tac.arg2);
        std::string dst = mapToRegister(tac.result);
        outfile << "    and " << dst << ", " << reg1 << ", " << reg2 << "\n";
      } else if (tac.op == "|") {
        // Division
        std::string reg1 = ensureLoaded(tac.arg1);
        std::string reg2 = ensureLoaded(tac.arg2);
        std::string dst = mapToRegister(tac.result);
        outfile << "    or " << dst << ", " << reg1 << ", " << reg2 << "\n";
      } else if (tac.op == "^") {
        // Division
        std::string reg1 = ensureLoaded(tac.arg1);
        std::string reg2 = ensureLoaded(tac.arg2);
        std::string dst = mapToRegister(tac.result);
        outfile << "    xor " << dst << ", " << reg1 << ", " << reg2 << "\n";
      } else if (tac.op == "<<") {
        // Division
        std::string reg1 = ensureLoaded(tac.arg1);
        std::string reg2 = ensureLoaded(tac.arg2);
        std::string dst = mapToRegister(tac.result);
        outfile << "    sll " << dst << ", " << reg1 << ", " << reg2 << "\n";
      } else if (tac.op == ">>") {
        // Division
        std::string reg1 = ensureLoaded(tac.arg1);
        std::string reg2 = ensureLoaded(tac.arg2);
        std::string dst = mapToRegister(tac.result);
        outfile << "    srl " << dst << ", " << reg1 << ", " << reg2 << "\n";
      } else if (tac.op == "&&") {
        // Division
        std::string reg1 = ensureLoaded(tac.arg1);
        std::string reg2 = ensureLoaded(tac.arg2);
        std::string dst = mapToRegister(tac.result);
        outfile << "    and " << dst << ", " << reg1 << ", " << reg2 << "\n";
      } else if (tac.op == "||") {
        // Division
        std::string reg1 = ensureLoaded(tac.arg1);
        std::string reg2 = ensureLoaded(tac.arg2);
        std::string dst = mapToRegister(tac.result);
        outfile << "    or " << dst << ", " << reg1 << ", " << reg2 << "\n";
      } else if (tac.op == "==") {
        // Division
        outfile << "    seqz " << mapToRegister(tac.result) << ", "
                << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2)
                << "\n";
      } else if (tac.op == "!=") {
        // Division
        outfile << "    snez " << mapToRegister(tac.result) << ", "
                << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2)
                << "\n";
      } else if (tac.op == "<") {
        // Division
        outfile << "    slt " << mapToRegister(tac.result) << ", "
                << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2)
                << "\n";
      } else if (tac.op == ">") {
        // Division
        outfile << "    slt " << mapToRegister(tac.result) << ", "
                << mapToRegister(tac.arg2) << ", " << mapToRegister(tac.arg1)
                << "\n";
      } else if (tac.op == "<=") {
        // Division
        outfile << "    slt " << mapToRegister(tac.result) << ", "
                << mapToRegister(tac.arg2) << ", " << mapToRegister(tac.arg1)
                << "\n";
        outfile << "    xori " << mapToRegister(tac.result) << ", "
                << mapToRegister(tac.result) << ", 1\n";
      } else if (tac.op == ">=") {
        // Division
        outfile << "    slt " << mapToRegister(tac.result) << ", "
                << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2)
                << "\n";
        outfile << "    xori " << mapToRegister(tac.result) << ", "
                << mapToRegister(tac.result) << ", 1\n";
      } else if (tac.op == "move") {
        // Move value
        outfile << "    mv " << mapToRegister(tac.result) << ", "
                << mapToRegister(tac.arg1) << "\n";
      } else if (tac.op == "~") {
        // Move value
        outfile << "    not " << mapToRegister(tac.result) << ", "
                << mapToRegister(tac.arg1) << "\n";
      } else if (tac.op == "=") {
        std::string srcReg = ensureLoaded(tac.arg1);
        std::string type = "long";
        int size = 8;

        if (varMap.count(tac.arg1))
          type = varMap[tac.arg1].type;

        if (varMap.count(tac.result) == 0) {
          size = (type == "int") ? 4 : 8;
          stackOffset -= size;
          varMap[tac.result] = {stackOffset, type};
        }

        std::string op = (type == "int") ? "sw" : "sd";

        // Always use s0-relative offset for locals
        outfile << "    " << op << " " << srcReg << ", "
                << varMap[tac.result].offset << "(s0)\n";
      }

      else if (tac.op == "seq") {
        // Move value
        outfile << "    seqz " << mapToRegister(tac.result) << ", "
                << mapToRegister(tac.arg1) << "\n";
      } else if (tac.op == "NEG") {
        // Move value
        outfile << "    neg " << mapToRegister(tac.result) << ", "
                << mapToRegister(tac.arg1) << "\n";
      } else if (tac.op == "beqz") {
        // Move value
        outfile << "    beqz " << mapToRegister(tac.arg1) << ", " << tac.arg2
                << "\n";
      } else if (tac.op == "bnez") {
        // Move value
        outfile << "    bnez " << mapToRegister(tac.arg1) << ", " << tac.arg2
                << "\n";
      } else if (tac.op == "beq") {
        // Move value
        outfile << "    beq " << mapToRegister(tac.arg1) << ", "
                << mapToRegister(tac.arg2) << ", " << tac.result << "\n";
      } else if (tac.op == "bne") {
        // Move value
        outfile << "    bne " << mapToRegister(tac.arg1) << ", "
                << mapToRegister(tac.arg2) << ", " << tac.result << "\n";
      } else if (tac.op == "blt") {
        outfile << "    blt " << mapToRegister(tac.arg1) << ", "
                << mapToRegister(tac.arg2) << ", " << tac.result << "\n";
      } else if (tac.op == "bgt") {
        outfile << "    blt " << mapToRegister(tac.arg2) << ", "
                << mapToRegister(tac.arg1) << ", " << tac.result << "\n";
      } else if (tac.op == "bge") {
        outfile << "    bge " << mapToRegister(tac.arg1) << ", "
                << mapToRegister(tac.arg2) << ", " << tac.result << "\n";
      } else if (tac.op == "ble") {
        outfile << "    bge " << mapToRegister(tac.arg2) << ", "
                << mapToRegister(tac.arg1) << ", " << tac.result << "\n";
      } else if (tac.op == "jmp") {
        // Move value
        outfile << "    j " << tac.result << "\n";
      } else if (tac.op == "label") {
        // Move value
        outfile << tac.arg1 << ":\n";
      } else if (tac.op == "call") {
        // Emit argument register moves in order
        for (size_t i = 0; i < pendingArgs.size(); ++i) {
          std::string argReg = "a" + std::to_string(i);
          outfile << "    mv " << argReg << ", "
                  << mapToRegister(pendingArgs[i]) << "\n";
        }

        // Emit the call
        outfile << "    call " << tac.arg1 << "\n";

        // Handle return value
        if (!tac.result.empty()) {
          outfile << "    mv " << mapToRegister(tac.result) << ", a0\n";
        }

        pendingArgs.clear(); // Reset for next call
      } else if (tac.op == "arg") {
        pendingArgs.push_back(tac.arg1); // Save argument for later use
      } else if (tac.op == "param") {
        if (varMap.find(tac.arg1) == varMap.end()) {
          // Default to long (8 bytes) unless type info is explicitly available
          int size = 8;
          std::string type = "long";

          stackOffset -= size;
          varMap[tac.arg1] = {stackOffset, type};
        }

        std::string reg = mapToArgRegister(tac.arg1, paramIndex++);
        std::string op = (varMap[tac.arg1].type == "int") ? "sw" : "sd";
        outfile << "    " << op << " " << reg << ", " << varMap[tac.arg1].offset
                << "(s0)\n";
      } else if (tac.op == "SignExtend") {
        std::string src = ensureLoaded(tac.arg1);
        std::string dst = mapToRegister(tac.result);
        outfile << "    sext.w " << dst << ", " << src
                << "\n"; // sign-extend 32-bit to 64-bit
      } else if (tac.op == "Truncate") {
        std::string src = ensureLoaded(tac.arg1);
        std::string dst = mapToRegister(tac.result);
        outfile << "    addiw " << dst << ", " << src
                << ", 0\n"; // truncates to 32-bit
      }
    }
  }
};

inline int TACtoASM::paramIndex = 0;