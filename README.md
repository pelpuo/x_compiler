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
* Increment and Decrement operator
* Switch
* Optimizations
    - Fold conditional stmt
    - Constant folding
