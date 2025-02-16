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
  cout << "(Parser) Unexpected: " << TokenStr[token.type] << " " << token.value.value_or("") << " on line " << token.line << "\n";
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

int Parser::getPrecedence(TokenType op){
    switch(op){
      case TokenType::LOGICAL_NOT:
        return 14;
      case TokenType::MUL:
      case TokenType::DIV:
      case TokenType::MOD:
        return 13;
      case TokenType::PLUS:
      case TokenType::MINUS:
        return 12;
      case TokenType::LEFT_SHIFT:
      case TokenType::RIGHT_SHIFT:
        return 11;
      case TokenType::LESS_THAN:
      case TokenType::LESS_THAN_EQUAL:
      case TokenType::GREATER_THAN:
      case TokenType::GREATER_THAN_EQUAL:
        return 10;
      case TokenType::EQUAL_EQUAL:
      case TokenType::NOT_EQUAL:
        return 9;
      case TokenType::BITWISE_AND:
        return 8;
      case TokenType::BITWISE_XOR:
        return 7;
      case TokenType::BITWISE_OR:
        return 6;
      case TokenType::LOGICAL_AND:
        return 5;
      case TokenType::LOGICAL_OR:
        return 4;
      case TokenType::EQUALS:
        return 2;
      default:
        return 0;
    }
}

bool Parser::isBinaryOp(TokenType type){
  switch(type){
    case TokenType::PLUS:
    case TokenType::MINUS:
    case TokenType::MUL:
    case TokenType::DIV:
    case TokenType::MOD:
    case TokenType::BITWISE_AND:
    case TokenType::BITWISE_OR:
    case TokenType::BITWISE_XOR:
    case TokenType::LOGICAL_AND:
    case TokenType::LOGICAL_OR:
    case TokenType::LOGICAL_NOT:
    case TokenType::LEFT_SHIFT:
    case TokenType::RIGHT_SHIFT:
    case TokenType::EQUALS:
    case TokenType::EQUAL_EQUAL:
    case TokenType::NOT_EQUAL:
    case TokenType::LESS_THAN:
    case TokenType::LESS_THAN_EQUAL:
    case TokenType::GREATER_THAN:
    case TokenType::GREATER_THAN_EQUAL:
      return true;
    default:
      return false;
  }
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

Func *Parser::parseFunction(){
  // std::unique_ptr<Block> stmts;
  std::unique_ptr<Block> stmts = std::make_unique<Block>();

  consume(TokenType::INT);
  expect(TokenType::ID);
  string name = token.value.value();
  consume(TokenType::ID);

  consume(TokenType::LEFT_PAREN);

  consume(TokenType::VOID);

  consume(TokenType::RIGHT_PAREN);

  consume(TokenType::LEFT_BRACE);
  BlockItem *nextItem;
  while (token.type != TokenType::RIGHT_BRACE) {
    nextItem = parseBlockItem();
    if(nextItem == nullptr)break;
    stmts->addItem(std::unique_ptr<BlockItem>(nextItem));
  }
  // stmts->addItem(std::unique_ptr<Stmt>(stmt));
  // consume(TokenType::SEMICOLON);
  consume(TokenType::RIGHT_BRACE);
  
  return new Func(name, std::move(stmts));
}


BlockItem *Parser::parseBlockItem() {
  // Check if the next token is a type specifier (indicating a declaration)
  if (token.type == TokenType::INT) {
    return parseDeclaration();
  } else {
    return parseStatement();
  }
}


Decl *Parser::parseDeclaration() {
  consume(TokenType::INT); // Consume 'int' keyword
  expect(TokenType::ID);
  
  std::string varName = token.value.value();
  consume(TokenType::ID);
  
  std::unique_ptr<Expr> initializer = nullptr;
  
  // Check for optional initialization (e.g., int x = 10;)
  if (token.type == TokenType::EQUALS) {
    consume(TokenType::EQUALS);
    initializer.reset(parseExpr());
  }
  
  consume(TokenType::SEMICOLON); // Expect a semicolon at the end
  return new Decl(varName, std::move(initializer));
}

Stmt *Parser::parseStatement(){
  if (token.type == TokenType::RETURN) {
    consume(TokenType::RETURN);
    Expr *expr = parseExpr();
    std::unique_ptr<Expr> retPtr(expr);
    consume(TokenType::SEMICOLON);
    return new ReturnStmt(std::move(retPtr));
  }
  else if(token.type == TokenType::ID){
    Expr *expr = parseExpr();
    consume(TokenType::SEMICOLON);
    return new ExprStmt(std::unique_ptr<Expr>(expr));
  }
  return nullptr;
}

Expr *Parser::parseExpr(int minPrec)
{
  Expr *left;

  if(token.type == TokenType::LOGICAL_NOT 
    || token.type == TokenType::MINUS 
    || token.type == TokenType::COMPLEMENT){
    TokenType op = token.type;
    int prec = getPrecedence(op);
    advance();
    Expr *right = parseExpr(getPrecedence(op)+1);
    left = new UnaryOp(op, std::unique_ptr<Expr>(right));
  }else{
    left = parseFactor();
  }

  while (isBinaryOp(token.type) && getPrecedence(token.type) >= minPrec)
  {
    TokenType op = token.type;
    int prec = getPrecedence(op);
    advance();
    
    if(op == TokenType::EQUALS){
      Expr *right = parseExpr(prec);
      left = new Assignment(std::unique_ptr<Expr>(left), std::unique_ptr<Expr>(right));
    }else{
      Expr *right = parseExpr(prec + 1);
      left = new BinaryOp(op, std::unique_ptr<Expr>(left), std::unique_ptr<Expr>(right));
    }
  
  }

  return left;
}

Expr *Parser::parseTerm()
{
    return nullptr;
}

Expr *Parser::parseFactor(){
  if(token.type == TokenType::NUM){
    string tokentext = token.value.value();
    consume(TokenType::NUM);
    return new IntLiteral(stoi(tokentext));
  }
  else if(token.type == TokenType::LEFT_PAREN){
    consume(TokenType::LEFT_PAREN);
    Expr *expr = parseExpr();
    consume(TokenType::RIGHT_PAREN);
    return expr;
  }else if(token.type == TokenType::ID){
    string name = token.value.value();
    consume(TokenType::ID);
    return new Variable(name);
  }
  else{
    error();
  }

}

Parser::Parser(Lexer &lex) : lexer(lex), HasError(false) { advance(); }