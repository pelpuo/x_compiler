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


```
riscv64-unknown-linux-gnu-as -o aprog.o aprog.S
riscv64-unknown-linux-gnu-gcc -o aprog aprog.o -march=rv64imafd -mabi=lp64d -static
```
