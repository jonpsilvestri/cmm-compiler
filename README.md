# C-- Compiler for MIPS32

A compiler for C-- to MIPS32

A description of the C-- Language can be found [here](CMM-LANGUAGE-SPEC.md)

## PLANNED, BUT UNSUPPORTED FEATURES

- arithmetic operations (+,-,/,*)
- while loops

## Features

- Lexical Analysis
- Syntax Analysis
- Semantic Checking
- Abstract Syntax Tree Generation
- Intermediate Code Generation
- Arithmetic Expression Evaluation (coming soon)
- MIPS32 Assembly Code Generation

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

```--gen_3ac```: prints the intermediary 3 address code to stdout

The compiler can be run with all, none, or a combination of flags

### Run

```compile``` receives code from stdin. C-- code can be provided via the command line until and EOF token is entered.

```bash
./compile [--chk_decl] [--print_ast] [--gen_code][--gen_3ac]
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
