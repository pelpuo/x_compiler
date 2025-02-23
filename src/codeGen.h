#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include "AST.h"

class CodeGenerator{
    public:
        CodeGenerator(ofstream &file) : outfile(file) {}
        void generate(ASTProgram *ast);

    private:
        ofstream &outfile;
        void generateFunction(FuncDecl *func);
        void generateStatement(Stmt *stmt);
        void generateAssign(IntLiteral *expr);
        void generateExpr(BinaryOp *expr);
        void generateReturn(Expr *expr);
};