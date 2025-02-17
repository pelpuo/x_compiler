#pragma once

#include "TAC.h"
#include <fstream>
#include <map>

class TACtoASM{
    private:
        std::ofstream &outfile;
        int tempVarCounter = 0; // Track temporary registers (t0, t1, ...)
        int stackOffset = 0;    // Track stack memory offset
        int stackSize = 64;     // Stack size for function prologue/epilogue

        std::map<std::string, int> varMap; // Maps variables to stack offsets
        std::map<std::string, std::string> registerMap; // Maps temp vars to RISC-V registers

        std::string getTempReg() {
            return "t" + std::to_string(tempVarCounter++ % 7); // Reuse t0-t6
        }

        std::string mapToRegister(const std::string &tempVar) {
            if (registerMap.find(tempVar) == registerMap.end()) {
                registerMap[tempVar] = getTempReg();
            }
            return registerMap[tempVar];
        }

        void emitPrologue(int stackSize = 64) {
            outfile << "    addi sp, sp, -" << stackSize << "\n";
            outfile << "    sd ra, " << (stackSize - 8) << "(sp)\n";
            outfile << "    sd s0, " << (stackSize - 16) << "(sp)\n";
            outfile << "    addi s0, sp, " << stackSize << "\n";
            stackOffset = 0; // Reset stack allocation
        }
    
        void emitEpilogue() {
            outfile << "    ld ra, " << (stackSize - 8) << "(sp)\n";
            outfile << "    ld s0, " << (stackSize - 16) << "(sp)\n";
            outfile << "    addi sp, sp, " << stackSize << "\n";
            outfile << "    ret\n";
        }
         

    public:
        TACtoASM(ofstream &file) : outfile(file) {}
        void generateAssembly(const std::vector<TAC>& tacCode) {
            outfile << ".text\n";
            outfile << ".globl main\n";
            outfile << "main:\n";

            emitPrologue();

            for (const auto& tac : tacCode) {
                if (tac.op == "var") {
                    // Allocate space for the variable
                    if (varMap.find(tac.result) == varMap.end()) {
                        varMap[tac.result] = stackOffset;
                        stackOffset -= 8; // Reserve 8 bytes for each variable
                    }
                }
                else if (tac.op == "li") {
                    // Load immediate
                    outfile << "    li " << mapToRegister(tac.result) << ", " << tac.arg1 << "\n";
                }
                else if (tac.op == "+") {
                    // Addition
                    outfile << "    add " << mapToRegister(tac.result) << ", " << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2) << "\n";
                }
                else if (tac.op == "-") {
                    // Subtraction
                    outfile << "    sub " << mapToRegister(tac.result) << ", " << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2) << "\n";
                }
                else if (tac.op == "*") {
                    // Multiplication
                    outfile << "    mul " << mapToRegister(tac.result) << ", " << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2) << "\n";
                }
                else if (tac.op == "/") {
                    // Division
                    outfile << "    div " << mapToRegister(tac.result) << ", " << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2) << "\n";
                }
                else if (tac.op == "%") {
                    // Division
                    outfile << "    rem " << mapToRegister(tac.result) << ", " << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2) << "\n";
                }
                else if (tac.op == "move") {
                    // Move value
                    outfile << "    mv " << mapToRegister(tac.result) << ", " << mapToRegister(tac.arg1) << "\n";
                }
                else if (tac.op == "~") {
                    // Move value
                    outfile << "    not " << mapToRegister(tac.result) << ", " << mapToRegister(tac.arg1) << "\n";
                }
                else if (tac.op == "seq") {
                    // Move value
                    outfile << "    seqz " << mapToRegister(tac.result) << ", " << mapToRegister(tac.arg1) << "\n";
                }
                else if (tac.op == "NEG") {
                    // Move value
                    outfile << "    neg " << mapToRegister(tac.result) << ", " << mapToRegister(tac.arg1) << "\n";
                }
                else if (tac.op == "beq") {
                    // Move value
                    outfile << "    beq " << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2) << ", " << tac.result << "\n";
                }
                else if (tac.op == "bne") {
                    // Move value
                    outfile << "    bne " << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2) << ", " << tac.result << "\n";
                }
                else if (tac.op == "blt"){
                    outfile << "    blt " << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2) << ", " << tac.result << "\n";
                    
                }
                else if (tac.op == "bgt"){
                    outfile << "    blt " << mapToRegister(tac.arg2) << ", " << mapToRegister(tac.arg1) << ", " << tac.result << "\n";
                    
                }
                else if (tac.op == "bge"){
                    outfile << "    bge " << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2) << ", " << tac.result << "\n";
                    
                }
                else if (tac.op == "ble"){
                    outfile << "    bge " << mapToRegister(tac.arg2) << ", " << mapToRegister(tac.arg1) << ", " << tac.result << "\n";
                    
                }
                else if (tac.op == "jmp") {
                    // Move value
                    outfile << "    j " << tac.result << "\n";
                }
                else if (tac.op == "label") {
                    // Move value
                    outfile << tac.arg1 << ":\n";
                }
                else if (tac.op == "store") {
                    // Store value from register into memory
                    if (varMap.find(tac.result) == varMap.end()) {
                        stackOffset -= 8;  // Allocate if not already allocated
                        varMap[tac.result] = stackOffset;
                    }
                    outfile << "    sd " << mapToRegister(tac.arg1) << ", " 
                            << varMap[tac.result] << "(s0)\n";
                }
                else if (tac.op == "load") {
                    // Load variable from memory
                    if (varMap.find(tac.arg1) != varMap.end()) {
                        int offset = varMap[tac.arg1];
                        outfile << "    ld " << mapToRegister(tac.result) << ", " << offset << "(s0)\n";
                    }
                }
                else if (tac.op == "call") {
                    // Call function
                    outfile << "    call " << tac.arg1 << "\n";
                }
                else if (tac.op == "RETURN") {
                    // Return
                    outfile << "    mv a0, " << mapToRegister(tac.arg1) << "\n";
                    // outfile << "    ret\n";
                    emitEpilogue();
                }
            }
        }
};