#pragma once

#include "lexer.h"
#include "AST.h"
#include "Type.h"
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
        bool isTypeSpecifier(TokenType type);

        ASTProgram *parseProgram();
        FuncDecl *parseFunction();
        Stmt *parseStatement();
        BlockItem *parseBlockItem();
        Block *parseBlock();
        Expr *parseExpr(int minPrec = 0);
        Expr *parseTerm();
        Expr *parseFactor();
        Expr *parseCast();
        Expr *parseConstant();
        Declaration *parseDeclaration();
        VarDecl *parseVarDecl(const string &varName, std::unique_ptr<Type> type);
        FuncDecl *parseFuncDecl(const string &funcName, std::unique_ptr<Type> returnType);
        FuncDecl *parseFuncDeclOrProto(const string &funcName, std::unique_ptr<Type> returnType);
        ExprStmt *parseExprStmt();
        TypeSpecifier parseType(std::vector<TypeSpecifier> &types);
        std::tuple<std::unique_ptr<Type>, StorageClass, TypeQualifier> parseTypeAndStorageClass();

        
    public:
        Parser(Lexer &lex);
        ASTProgram *parse();
};