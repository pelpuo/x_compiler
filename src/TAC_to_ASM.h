#pragma once

#include "TAC.h"
#include <fstream>
#include <map>

class TACtoASM{
    private:
        ofstream &outfile;
        int tempVarCounter = 0; // Used to keep track of temporary variables (t0, t1, ...)
        std::map<std::string, std::string> registerMap; // Map temporary vars to RISC-V registers

        std::string getTempReg() {
            return "t" + std::to_string(tempVarCounter++);
        }

        std::string mapToRegister(std::string tempVar) {
            if (registerMap.find(tempVar) == registerMap.end()) {
                registerMap[tempVar] = getTempReg();
            }
            return registerMap[tempVar];
        }

        void emitPrologue(int stackSize = 64){
            outfile << "    addi sp, sp, -" << stackSize << "\n";
            outfile << "    sd ra, " << (stackSize - 8) << "(sp)\n";
            outfile << "    sd s0, " << (stackSize - 16) << "(sp)\n";
            outfile << "    addi s0, sp, " << stackSize << "\n";
     
        }

        void emitEpilogue(){
            outfile << "    ld ra, 56(sp)\n";
            outfile << "    ld s0, 48(sp)\n";
            outfile << "    addi sp, sp, 64\n";
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
                if (tac.op == "li") {
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
                    outfile << "    beq " << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2) << ", " << mapToRegister(tac.result) << "\n";
                }
                else if (tac.op == "bne") {
                    // Move value
                    outfile << "    bne " << mapToRegister(tac.arg1) << ", " << mapToRegister(tac.arg2) << ", " << mapToRegister(tac.result) << "\n";
                }
                else if (tac.op == "jmp") {
                    // Move value
                    outfile << "    j " << mapToRegister(tac.result) << "\n";
                }
                else if (tac.op == "label") {
                    // Move value
                    outfile << tac.arg1 << ":\n";
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