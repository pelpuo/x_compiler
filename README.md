# Dummy Compiler

```
bison--defines=token.h--output=parser.c parser.bison
```

```
flex -o scanner.c scanner.flex
```


<program> ::= <function>
<function> ::= "int" <identifier> "(" "void" ")" "{" <statement> "}"
<statement> ::= "return" <exp> ";"
<exp> ::= <int> | <unop> <exp> | "(" <exp> ")"
<unop> ::= "-" | "~"
<identifier> ::= ? An identifier token ?
<int> ::= ? A constant token ?
 

### Todos
* Error Handling on AST Generation
* Code Generation
* Unary Expressions
* Binary Expressions
* Symbol Table
* Control Flow
    - If Stmt
    - While loop
    - For Loop
* Scopes
