# C-- Compiler for MIPS32

A compiler for C-- to MIPS32

## Features

- Lexical Analysis
- Syntax Analysis
- Semantic Checking
- Abstract Syntax Tree Generation (coming soon)
- Intermediate Code Generation (coming soon)
- Arithmetic Expression Evaluation (coming soon)
- MIPS32 Assembly Code Generation (coming soon)

### Installation

Clone the repository and make the compiler:

```bash
make
```
The resulting excecutable will be called ```compile```

### Optional Flags
There are 4 optional flags for compilation

```--chk_decl```: parser will perfom semantic checking, and throw a syntax error if one is encounters (e.g. multiply declared variables)

```--print_ast```: prints out the AST structure of the inputted source code to stdout

```--gen_code```: prints MIPS32 assembly code of the source code to stdout

```--print_3ac```: prints the intermediary 3 address code to stdout

The compiler can be run with all, none, or a combination of flags

### Run

```compile``` receives code from stdin. C-- code can be provided via the command line until and EOF token is entered.

```bash
./compile [--chk_decl] [--print_ast] [--gen_code][--print_3ac]
```

to pipe input from a file, and produce a .s file:
```bash
cat [filename] | ./compile --gen_code > output.s
```

### Clean
to clean all libraries and excecutables:

```bash
make clean
```

# C-- Language Specification

## Tokens

| Name      | Pattern                          | Comments                        |
|-----------|----------------------------------|---------------------------------|
| ID        | `letter { letter \| digit \| _ }`| identifier: e.g., x, abc, p_q_12|
| INTCON    | `digit { digit }`                | integer constant: e.g., 12345   |
| LPAREN    | `(`                              | Left parenthesis                |
| RPAREN    | `)`                              | Right parenthesis               |
| LBRACE    | `{`                              | Left curly brace                |
| RBRACE    | `}`                              | Right curly brace               |
| COMMA     | `,`                              | Comma                           |
| SEMI      | `;`                              | Semicolon                       |
| kwINT     | `int`                            | Keyword: int                    |
| kwIF      | `if`                             | Keyword: if                     |
| kwELSE    | `else`                           | Keyword: else                   |
| kwWHILE   | `while`                          | Keyword: while                  |
| kwRETURN  | `return`                         | Keyword: return                 |
| opASSG    | `=`                              | Op: Assignment                  |
| opADD     | `+`                              | Op: addition                    |
| opSUB     | `–`                              | Op: subtraction                 |
| opMUL     | `*`                              | Op: multiplication              |
| opDIV     | `/`                              | Op: division                    |
| opEQ      | `==`                             | Op: equals                      |
| opNE      | `!=`                             | Op: not-equals                  |
| opGT      | `>`                              | Op: greater-than                |
| opGE      | `>=`                             | Op: greater than or equal       |
| opLT      | `<`                              | Op: less-than                   |
| opLE      | `<=`                             | Op: less than or equal          |
| opAND     | `&&`                             | Op: logical and                |
| opOR      | `\|\|`                           | Op: logical or                 |



