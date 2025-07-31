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


### Bug fixes
* static functions
* extern
* Type checking usage

* global variables
* Data Types
* Data Structures
* Register Allocation


```
[DEBUG] Resolving BlockItem...
Declaration: int a = ConstInt: 12

[DEBUG] Resolving symbol a
[DEBUG] Resolving BlockItem...
Declaration: long b = ConstInt: 32

[DEBUG] Resolving symbol b
[DEBUG] Resolving BlockItem...
ReturnStmt: Variable: b
-------------------------------------
[SymbolTable] Declaring 'a'
[SymbolTable] Declaring 'b'
Typechecked Variable: b, type: long
-------------------------------------
Function Declaration: main()
Declaration: int a = ConstInt: 12

Declaration: long b = Cast: (long) ConstInt: 32

ReturnStmt: Variable: b
 = main function 1
t0 = 12 li int
a = t0 store int
t1 = 32 li int
t2 = SignExtend t1
b = t2 store long
t3 = b load long
 = RETURN t3

Executing Program...
Return :  32
```