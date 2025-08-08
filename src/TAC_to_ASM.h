#pragma once

#include "TAC.h"
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

class TACtoASM {
private:
    std::ofstream &outfile;
    int tempVarCounter = 0;
    int floatTempVarCounter = 0;
    int stackOffset = 0;
    int stackSize = 128; // Default stack size
    static int intParamIndex;
    static int floatParamIndex;
    static int dataLabelCounter;

    struct VarInfo {
        int offset;
        std::string type; // "int", "long", "double", "unsigned_int", etc.
    };

    std::map<std::string, VarInfo> varMap;
    std::map<std::string, std::string> registerMap; // For integer temp vars
    std::map<std::string, std::string> floatRegisterMap; // For float temp vars
    std::map<std::string, std::string> floatConstantLabels; // Maps float literal to its .rodata label
    
    std::vector<std::pair<std::string, std::string>> pendingArgs;

    // --- Type and Register Helpers ---

    bool isFloatType(const std::string& typeStr) {
        return typeStr == "double";
    }

    std::string getTempReg() {
        return "t" + std::to_string(tempVarCounter++ % 7); // Reuse t0-t6
    }

    std::string getFloatTempReg() {
        return "ft" + std::to_string(floatTempVarCounter++ % 8); // Reuse ft0-ft7
    }

    std::string mapToRegister(const std::string &tempVar) {
        if (registerMap.find(tempVar) == registerMap.end()) {
            registerMap[tempVar] = getTempReg();
        }
        return registerMap[tempVar];
    }

    std::string mapToFloatRegister(const std::string &tempVar) {
        if (floatRegisterMap.find(tempVar) == floatRegisterMap.end()) {
            floatRegisterMap[tempVar] = getFloatTempReg();
        }
        return floatRegisterMap[tempVar];
    }
    
    bool isImmediate(const std::string &s) {
        if (s.empty()) return false;
        char* p;
        strtol(s.c_str(), &p, 10);
        return *p == 0;
    }
    
    std::string ensureLoaded(const std::string &name) {
        if (isImmediate(name)) {
            std::string tempReg = getTempReg();
            outfile << "    li " << tempReg << ", " << name << "\n";
            return tempReg;
        }

        if (varMap.count(name)) {
            std::string reg = mapToRegister(name);
            const auto& info = varMap[name];
            std::string op = (info.type == "int" || info.type == "unsigned_int") ? "lw" : "ld";
            outfile << "    " << op << " " << reg << ", " << info.offset << "(s0)\n";
            return reg;
        }
        
        return mapToRegister(name);
    }

    std::string ensureFloatLoaded(const std::string &name) {
        if (varMap.count(name)) {
            std::string reg = mapToFloatRegister(name);
            outfile << "    fld " << reg << ", " << varMap[name].offset << "(s0)\n";
            return reg;
        }
        return mapToFloatRegister(name);
    }

    // --- Emitter Functions ---

    void emitGlobalData(const std::vector<TAC> &dataTACs) {
        bool dataHeaderEmitted = false;
        for (const auto &tac : dataTACs) {
            if (tac.op == "StaticVariable") {
                if (!dataHeaderEmitted) {
                    outfile << ".data\n";
                    dataHeaderEmitted = true;
                }
                const std::string &name = tac.arg1;
                const std::string &type = tac.arg2;
                if (tac.result == "-1") {
                    outfile << ".extern " << name << "\n";
                } else {
                    outfile << ".globl " << name << "\n";
                    outfile << name << ":\n";
                    if (type == "int" || type == "unsigned_int") outfile << "    .word " << tac.result << "\n";
                    else if (type == "long" || type == "unsigned_long") outfile << "    .quad " << tac.result << "\n";
                    else if (isFloatType(type)) outfile << "    .double " << tac.result << "\n";
                }
            }
        }
    }

    void emitPrologue(int stackSize) {
        outfile << "    addi sp, sp, -" << stackSize << "\n";
        outfile << "    sd ra, " << (stackSize - 8) << "(sp)\n";
        outfile << "    sd s0, " << (stackSize - 16) << "(sp)\n";
        outfile << "    addi s0, sp, " << stackSize << "\n";
        stackOffset = -24;
    }

    void emitEpilogue(int stackSize) {
        outfile << "    ld ra, " << (stackSize - 8) << "(sp)\n";
        outfile << "    ld s0, " << (stackSize - 16) << "(sp)\n";
        outfile << "    addi sp, sp, " << stackSize << "\n";
        outfile << "    ret\n";
    }

public:
    TACtoASM(std::ofstream &file) : outfile(file) {}

    void generateAssembly(const std::vector<TAC> &tacCode) {
        std::vector<TAC> dataTACs, codeTACs;
        for (const auto &tac : tacCode) {
            if (tac.op == "StaticVariable") dataTACs.push_back(tac);
            else codeTACs.push_back(tac);
        }

        // Emit .data section for global/static variables
        emitGlobalData(dataTACs);

        // **NEW**: Pre-scan for all float constants and create labels for them
        for (const auto &tac : codeTACs) {
            if (tac.op == "li" && isFloatType(tac.arg2)) {
                if (floatConstantLabels.find(tac.arg1) == floatConstantLabels.end()) {
                    std::string dataLabel = ".LD" + std::to_string(dataLabelCounter++);
                    floatConstantLabels[tac.arg1] = dataLabel;
                }
            }
        }

        // **NEW**: Emit one consolidated .rodata section for all float constants
        if (!floatConstantLabels.empty()) {
            outfile << ".section .rodata\n";
            for (const auto& pair : floatConstantLabels) {
                outfile << pair.second << ":\n"; // The label (e.g., .LD0)
                outfile << "    .double " << pair.first << "\n"; // The value (e.g., 1.500000)
            }
        }
        
        // Start the .text section for all executable code
        outfile << ".text\n";

        for (const auto &tac : codeTACs) {
            // outfile << "# " << tac.op << " " << tac.result << ", " << tac.arg1 << ", " << tac.arg2 << "\n";

            // Function Definition
            if (tac.op == "function") {
                outfile << ".globl " << tac.arg1 << "\n";
                outfile << ".type " << tac.arg1 << ", @function\n";
                outfile << tac.arg1 << ":\n";
                tempVarCounter = 0; floatTempVarCounter = 0;
                varMap.clear(); registerMap.clear(); floatRegisterMap.clear();
                intParamIndex = 0; floatParamIndex = 0;
                emitPrologue(stackSize);
            }
            // Return Statement
            else if (tac.op == "RETURN") {
                if (floatRegisterMap.count(tac.arg1)) {
                    outfile << "    fmv.d fa0, " << ensureFloatLoaded(tac.arg1) << "\n";
                } else {
                    outfile << "    mv a0, " << ensureLoaded(tac.arg1) << "\n";
                }
                emitEpilogue(stackSize);
            }
            // Memory Operations (Store/Load)
            else if (tac.op == "store") {
                const std::string& type = tac.arg2;
                if (varMap.find(tac.result) == varMap.end()) {
                    int size = isFloatType(type) ? 8 : (type == "int" || type == "unsigned_int" ? 4 : 8);
                    stackOffset -= size;
                    varMap[tac.result] = {stackOffset, type};
                }
                if (isFloatType(type)) {
                    outfile << "    fsd " << ensureFloatLoaded(tac.arg1) << ", " << varMap[tac.result].offset << "(s0)\n";
                } else {
                    std::string op = (type == "int" || type == "unsigned_int") ? "sw" : "sd";
                    outfile << "    " << op << " " << ensureLoaded(tac.arg1) << ", " << varMap[tac.result].offset << "(s0)\n";
                }
            }
            else if (tac.op == "load") {
                const std::string& type = tac.arg2;
                if (isFloatType(type)) {
                    std::string dstReg = mapToFloatRegister(tac.result);
                    if (varMap.count(tac.arg1)) outfile << "    fld " << dstReg << ", " << varMap[tac.arg1].offset << "(s0)\n";
                    else { outfile << "    la t6, " << tac.arg1 << "\n"; outfile << "    fld " << dstReg << ", 0(t6)\n"; }
                } else {
                    std::string dstReg = mapToRegister(tac.result);
                    std::string op = (type == "int" || type == "unsigned_int") ? "lw" : "ld";
                    if (varMap.count(tac.arg1)) outfile << "    " << op << " " << dstReg << ", " << varMap[tac.arg1].offset << "(s0)\n";
                    else { outfile << "    la t6, " << tac.arg1 << "\n"; outfile << "    " << op << " " << dstReg << ", 0(t6)\n"; }
                }
            }
            // Load Immediate
            else if (tac.op == "li") {
                // **NEW**: The li handler is now much simpler
                if(isFloatType(tac.arg2)) {
                    // Just look up the label and load from it
                    std::string dataLabel = floatConstantLabels[tac.arg1];
                    outfile << "    la t6, " << dataLabel << "\n";
                    outfile << "    fld " << mapToFloatRegister(tac.result) << ", 0(t6)\n";
                } else {
                    outfile << "    li " << mapToRegister(tac.result) << ", " << tac.arg1 << "\n";
                }
            }
            // Arithmetic Operations
            else if (tac.op == "+" || tac.op == "-" || tac.op == "*" || tac.op == "/") {
                if (floatRegisterMap.count(tac.arg1) || floatRegisterMap.count(tac.arg2)) {
                    std::map<std::string, std::string> opMap = {{"+", "fadd.d"}, {"-", "fsub.d"}, {"*", "fmul.d"}, {"/", "fdiv.d"}};
                    outfile << "    " << opMap[tac.op] << " " << mapToFloatRegister(tac.result) << ", " << ensureFloatLoaded(tac.arg1) << ", " << ensureFloatLoaded(tac.arg2) << "\n";
                } else {
                    std::map<std::string, std::string> opMap = {{"+", "add"}, {"-", "sub"}, {"*", "mul"}, {"/", "div"}};
                    if(isImmediate(tac.arg2) && tac.op == "+") outfile << "    addi " << mapToRegister(tac.result) << ", " << ensureLoaded(tac.arg1) << ", " << tac.arg2 << "\n";
                    else outfile << "    " << opMap[tac.op] << " " << mapToRegister(tac.result) << ", " << ensureLoaded(tac.arg1) << ", " << ensureLoaded(tac.arg2) << "\n";
                }
            }
            // Integer-only Bitwise and Remainder
            else if (tac.op == "%" || tac.op == "&" || tac.op == "|" || tac.op == "^" || tac.op == "<<" || tac.op == ">>") {
                std::map<std::string, std::string> opMap = {{"%", "rem"}, {"&", "and"}, {"|", "or"}, {"^", "xor"}, {"<<", "sll"}, {">>", "sra"}};
                outfile << "    " << opMap[tac.op] << " " << mapToRegister(tac.result) << ", " << ensureLoaded(tac.arg1) << ", " << ensureLoaded(tac.arg2) << "\n";
            }
            // Comparison Operations
            else if (tac.op == "==" || tac.op == "!=" || tac.op == "<" || tac.op == ">" || tac.op == "<=" || tac.op == ">=") {
                 if(floatRegisterMap.count(tac.arg1) || floatRegisterMap.count(tac.arg2)) {
                    std::string dst = mapToRegister(tac.result);
                    std::string freg1 = ensureFloatLoaded(tac.arg1);
                    std::string freg2 = ensureFloatLoaded(tac.arg2);
                    if(tac.op == "==") outfile << "    feq.d " << dst << ", " << freg1 << ", " << freg2 << "\n";
                    else if(tac.op == "!=") { outfile << "    feq.d " << dst << ", " << freg1 << ", " << freg2 << "\n"; outfile << "    seqz " << dst << ", " << dst << "\n"; }
                    else if(tac.op == "<") outfile << "    flt.d " << dst << ", " << freg1 << ", " << freg2 << "\n";
                    else if(tac.op == "<=") outfile << "    fle.d " << dst << ", " << freg1 << ", " << freg2 << "\n";
                    else if(tac.op == ">") outfile << "    flt.d " << dst << ", " << freg2 << ", " << freg1 << "\n";
                    else if(tac.op == ">=") outfile << "    fle.d " << dst << ", " << freg2 << ", " << freg1 << "\n";
                 } else {
                    std::string dst = mapToRegister(tac.result);
                    std::string reg1 = ensureLoaded(tac.arg1);
                    std::string reg2 = ensureLoaded(tac.arg2);
                    if(tac.op == "==") { outfile << "    sub " << dst << ", " << reg1 << ", " << reg2 << "\n"; outfile << "    seqz " << dst << ", " << dst << "\n"; }
                    else if(tac.op == "!=") { outfile << "    sub " << dst << ", " << reg1 << ", " << reg2 << "\n"; outfile << "    snez " << dst << ", " << dst << "\n"; }
                    else if(tac.op == "<") outfile << "    slt " << dst << ", " << reg1 << ", " << reg2 << "\n";
                    else if(tac.op == ">") outfile << "    slt " << dst << ", " << reg2 << ", " << reg1 << "\n";
                    else if(tac.op == "<=") { outfile << "    slt " << dst << ", " << reg2 << ", " << reg1 << "\n"; outfile << "    xori " << dst << ", " << dst << ", 1\n"; }
                    else if(tac.op == ">=") { outfile << "    slt " << dst << ", " << reg1 << ", " << reg2 << "\n"; outfile << "    xori " << dst << ", " << dst << ", 1\n"; }
                 }
            }
            // Unary and Move Operations
            else if (tac.op == "move" || tac.op == "Copy") outfile << "    mv " << mapToRegister(tac.result) << ", " << ensureLoaded(tac.arg1) << "\n";
            else if (tac.op == "NEG") outfile << "    neg " << mapToRegister(tac.result) << ", " << ensureLoaded(tac.arg1) << "\n";
            else if (tac.op == "~") outfile << "    not " << mapToRegister(tac.result) << ", " << ensureLoaded(tac.arg1) << "\n";
            else if (tac.op == "seq") outfile << "    seqz " << mapToRegister(tac.result) << ", " << ensureLoaded(tac.arg1) << "\n";
            // Control Flow
            else if (tac.op == "label") outfile << tac.arg1 << ":\n";
            else if (tac.op == "jmp") outfile << "    j " << tac.result << "\n";
            // Conditional Branches
            else if (tac.op == "beqz") outfile << "    beqz " << ensureLoaded(tac.arg1) << ", " << tac.arg2 << "\n";
            else if (tac.op == "bnez") outfile << "    bnez " << ensureLoaded(tac.arg1) << ", " << tac.arg2 << "\n";
            else if (tac.op == "beq") outfile << "    beq " << ensureLoaded(tac.arg1) << ", " << ensureLoaded(tac.arg2) << ", " << tac.result << "\n";
            else if (tac.op == "bne") outfile << "    bne " << ensureLoaded(tac.arg1) << ", " << ensureLoaded(tac.arg2) << ", " << tac.result << "\n";
            else if (tac.op == "blt") outfile << "    blt " << ensureLoaded(tac.arg1) << ", " << ensureLoaded(tac.arg2) << ", " << tac.result << "\n";
            else if (tac.op == "bgt") outfile << "    bgt " << ensureLoaded(tac.arg1) << ", " << ensureLoaded(tac.arg2) << ", " << tac.result << "\n";
            else if (tac.op == "ble") outfile << "    ble " << ensureLoaded(tac.arg1) << ", " << ensureLoaded(tac.arg2) << ", " << tac.result << "\n";
            else if (tac.op == "bge") outfile << "    bge " << ensureLoaded(tac.arg1) << ", " << ensureLoaded(tac.arg2) << ", " << tac.result << "\n";
            // Function Call Machinery
            else if (tac.op == "arg") pendingArgs.push_back({tac.arg1, tac.arg2});
            else if (tac.op == "call") {
                int currentIntArg = 0, currentFloatArg = 0;
                for (const auto& arg : pendingArgs) {
                    if (isFloatType(arg.second)) {
                        if (currentFloatArg < 8) outfile << "    fmv.d fa" << currentFloatArg++ << ", " << ensureFloatLoaded(arg.first) << "\n";
                    } else {
                        if (currentIntArg < 8) outfile << "    mv a" << currentIntArg++ << ", " << ensureLoaded(arg.first) << "\n";
                    }
                }
                pendingArgs.clear();
                outfile << "    call " << tac.arg1 << "\n";
                if (!tac.result.empty()) {
                    if(isFloatType(tac.arg2)) outfile << "    fmv.d " << mapToFloatRegister(tac.result) << ", fa0\n";
                    else outfile << "    mv " << mapToRegister(tac.result) << ", a0\n";
                }
            }
            else if (tac.op == "param") {
                const std::string& type = tac.arg2;
                if (varMap.find(tac.arg1) == varMap.end()) {
                    int size = isFloatType(type) ? 8 : (type == "int" || type == "unsigned_int" ? 4 : 8);
                    stackOffset -= size; varMap[tac.arg1] = {stackOffset, type};
                }
                if (isFloatType(type)) outfile << "    fsd fa" << floatParamIndex++ << ", " << varMap[tac.arg1].offset << "(s0)\n";
                else {
                    std::string op = (type == "int" || type == "unsigned_int") ? "sw" : "sd";
                    outfile << "    " << op << " a" << intParamIndex++ << ", " << varMap[tac.arg1].offset << "(s0)\n";
                }
            }
            // Casting and Type Conversion
            else if (tac.op == "IntToDouble") outfile << "    fcvt.d.w " << mapToFloatRegister(tac.result) << ", " << ensureLoaded(tac.arg1) << "\n";
            else if (tac.op == "DoubleToInt") outfile << "    fcvt.w.d " << mapToRegister(tac.result) << ", " << ensureFloatLoaded(tac.arg1) << ", rtz\n";
            else if (tac.op == "SignExtend") outfile << "    sext.w " << mapToRegister(tac.result) << ", " << ensureLoaded(tac.arg1) << "\n";
            else if (tac.op == "Truncate") outfile << "    addiw " << mapToRegister(tac.result) << ", " << ensureLoaded(tac.arg1) << ", 0\n";
            else if (tac.op == "ZeroExtend") {
                std::string dst = mapToRegister(tac.result);
                std::string src = ensureLoaded(tac.arg1);
                outfile << "    slli " << dst << ", " << src << ", 32\n";
                outfile << "    srli " << dst << ", " << dst << ", 32\n";
            }
            else if (tac.op == "getAddress") {
                std::string dstReg = mapToRegister(tac.result);
                // If the variable is on the stack (local)
                if (varMap.count(tac.arg1)) {
                    outfile << "    lea " << dstReg << ", " << varMap[tac.arg1].offset << "(s0)\n";
                } 
                // Otherwise, it must be a global/static variable
                else {
                    outfile << "    la " << dstReg << ", " << tac.arg1 << "\n";
                }
            }
            else if (tac.op == "load_ptr") {
                const std::string& type = tac.arg2;
                std::string srcPtrReg = ensureLoaded(tac.arg1); // Register holding the address

                if (isFloatType(type)) {
                    std::string dstReg = mapToFloatRegister(tac.result);
                    outfile << "    fld " << dstReg << ", 0(" << srcPtrReg << ")\n";
                } else {
                    std::string dstReg = mapToRegister(tac.result);
                    std::string op = (type == "int" || type == "unsigned_int") ? "lw" : "ld";
                    outfile << "    " << op << " " << dstReg << ", 0(" << srcPtrReg << ")\n";
                }
            }
            else if (tac.op == "store_ptr") {
                const std::string& type = tac.arg2;
                std::string dstPtrReg = ensureLoaded(tac.result); // Register holding the address

                if (isFloatType(type)) {
                    std::string srcValReg = ensureFloatLoaded(tac.arg1);
                    outfile << "    fsd " << srcValReg << ", 0(" << dstPtrReg << ")\n";
                } else {
                    std::string srcValReg = ensureLoaded(tac.arg1); // Register with the value
                    std::string op = (type == "int" || type == "unsigned_int") ? "sw" : "sd";
                    outfile << "    " << op << " " << srcValReg << ", 0(" << dstPtrReg << ")\n";
                }
            }
            // Unhandled
            else {
                std::cerr << "FATAL: Unknown TAC operation in ASM generation: " << tac.op << "\n";
                exit(1);
            }
        }
    }
};

// Initialize static members
int TACtoASM::intParamIndex = 0;
int TACtoASM::floatParamIndex = 0;
int TACtoASM::dataLabelCounter = 0;