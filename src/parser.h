#pragma once

#include "lexer.h"
#include "AST.h"
#include <iostream>

using namespace std;

class Parser{
    private:
        Lexer &lexer;
        Token token;
        bool HasError;
        void advance();
        void error();
        bool expect(TokenType type);
        bool consume(TokenType type);
        int getPrecedence(TokenType op);
        bool isBinaryOp(TokenType type);
        bool isCompoundAssignOp(TokenType type);

        ASTProgram *parseProgram();
        FuncDecl *parseFunction();
        Stmt *parseStatement();
        BlockItem *parseBlockItem();
        Block *parseBlock();
        Expr *parseExpr(int minPrec = 0);
        Expr *parseTerm();
        Expr *parseFactor();
        Declaration *parseDeclaration();
        VarDecl *parseVarDecl(const string &varName);
        FuncDecl *parseFuncDecl(const string &funcName);
        ExprStmt *parseExprStmt();
        
    public:
        Parser(Lexer &lex);
        ASTProgram *parse();
};