#include "codeGen.h"

void CodeGenerator::generate(ASTProgram *ast) {
    outfile << ".option nopic" << endl 
            << ".attribute arch, \"rv64i2p0_m2p0_a2p0_f2p0_d2p0_c2p0\"" << endl 
            << ".attribute unaligned_access, 0" << endl
            << ".attribute stack_align, 16"<< endl
            << ".text\n.align 1"<< endl << endl;

    outfile << ".section .text" << endl 
            << ".globl _start" << endl
            << "_start:" << endl
            << "la sp, stack_top" << endl 
            << "call main" << endl
            << "li a7, 93   # syscall_exit" << endl 
            << "ecall" << endl;
    for(auto &func : ast->functions){
        generateFunction(func.get());
    }
}

void CodeGenerator::generateFunction(FuncDecl *func) {
    outfile << ".globl " << func->name  << endl
            << ".type " << func->name << ", @function" << endl
            << func->name << ":" << endl;

    // Prologue
    outfile << "addi sp, sp, -16" << endl
            << "sd ra, 8(sp)" << endl
            << "sd s0, 0(sp)" << endl
            << "addi s0, sp, 16" << endl;

    // for(auto &stmt: func->body->items){
    //     generateStatement(stmt.get());
    // }

    // Epilogue
    outfile  << "ld ra, 8(sp)" << endl
             << "ld s0, 0(sp)" << endl
             << "addi sp, sp, 16" << endl
             << "jr ra" << endl;

    outfile << "\n" 
            << ".section .bss" << endl 
            << ".align 12" << endl 
            << "stack_bottom:" << endl 
            << "    .skip 4096" << endl 
            << "stack_top:" << endl;
    
}

void CodeGenerator::generateStatement(Stmt *stmt) {
    switch(stmt->getType()){
        case StmtType::EXPR:
        break; 
        case StmtType::RETURN:
            generateReturn(dynamic_cast<ReturnStmt*>(stmt)->expr.get());
        break; 
        default:
        cout << "ERROR: Unknown Statement" << endl;
        break;
    }
}

void CodeGenerator::generateAssign(IntLiteral *expr) {

}
void CodeGenerator::generateExpr(BinaryOp *expr) {}
void CodeGenerator::generateReturn(Expr *expr) {
    outfile << "li t0, " << dynamic_cast<IntLiteral*>(expr)->value << endl;
    outfile << "mv a0, t0" << endl;
}
