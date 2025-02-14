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
    
        ASTProgram *parseProgram();
        Func *parseFunction();
        Stmt *parseStatement();
        Expr *parseExpr();
        Expr *parseTerm();
        Expr *parseFactor();
        
    public:
        Parser(Lexer &lex);
        ASTProgram *parse();
};