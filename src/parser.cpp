#include "parser.h"

void Parser::advance()
{
  lexer.next(token);
  if (token.type == TokenType::UNKNOWN)
  {
    error();
  }
}

bool Parser::expect(TokenType type)
{
  if (token.type != type)
  {
    error();
    return false;
  }
  return true;
}

void Parser::error()
{
  cout << "Unexpected: " << TokenStr[token.type] << " " << token.value.value_or("") << " on line " << token.line << "\n";
  HasError = true;
  exit(1);
}

bool Parser::consume(TokenType type)
{
  if (!expect(type))
    return false;
  advance();
  return true;
}

ASTProgram *Parser::parse()
{
  ASTProgram *Res = parseProgram();
  expect(TokenType::EOI);
  return Res;
}

ASTProgram *Parser::parseProgram()
{
  ASTProgram *program = new ASTProgram();
  Func *func;
  while ((func = parseFunction()))
  {
    program->addFunction(std::unique_ptr<Func>(func));
    if (expect(TokenType::EOI))
    {
      return program;
    }
  }
  return program;
}

Func *Parser::parseFunction()
{
  std::vector<std::unique_ptr<Stmt>> stmts;

  consume(TokenType::INT);
  expect(TokenType::ID);
  string name = token.value.value();
  consume(TokenType::ID);

  consume(TokenType::LEFT_PAREN);

  consume(TokenType::VOID);

  consume(TokenType::RIGHT_PAREN);

  consume(TokenType::LEFT_BRACE);
  Stmt *stmt = parseStatement();
  stmts.push_back(std::unique_ptr<Stmt>(stmt));
  consume(TokenType::SEMICOLON);
  consume(TokenType::RIGHT_BRACE);

  return new Func(name, std::move(stmts));
}

Stmt *Parser::parseStatement()
{
  if (consume(TokenType::RETURN))
  {
    Expr *expr = parseExpr();
    std::unique_ptr<Expr> retPtr(expr);
    return new ReturnStmt(std::move(retPtr));
  }
  return nullptr;
}

Expr *Parser::parseExpr()
{
  Expr *left = parseFactor();

  while (token.type == TokenType::PLUS 
    || token.type == TokenType::MINUS
    || token.type == TokenType::MUL
    || token.type == TokenType::MOD
    || token.type == TokenType::DIV)
  {
    TokenType op = token.type;
    advance();
    Expr *right = parseFactor();
    left = new BinaryOp(op, std::unique_ptr<Expr>(left), std::unique_ptr<Expr>(right));
  }

  return left;
}

Expr *Parser::parseTerm()
{
    return nullptr;
}

Expr *Parser::parseFactor()
{
  if(token.type == TokenType::NUM){
    string tokentext = token.value.value();
    consume(TokenType::NUM);
    return new IntLiteral(stoi(tokentext));
  }else if(token.type == TokenType::COMPLEMENT || token.type == TokenType::MINUS){
    advance();
    Expr *expr = parseExpr();
    return new UnaryOp(token.type, std::unique_ptr<Expr>(expr));
  }else if(token.type == TokenType::LEFT_PAREN){
    consume(TokenType::LEFT_PAREN);
    Expr *expr = parseExpr();
    consume(TokenType::RIGHT_PAREN);
    return expr;
  }else{
    error();
  }

}

Parser::Parser(Lexer &lex) : lexer(lex), HasError(false) { advance(); }