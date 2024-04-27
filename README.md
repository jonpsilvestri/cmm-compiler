# C-- Compiler for MIPS32

A compiler for C-- to MIPS32

A description of the C-- Language can be found [here](CMM-LANGUAGE-SPEC.md)

## PLANNED, BUT UNSUPPORTED FEATURES

- MIPS generation for compound boolean expressions AND & OR (ex: x > y || y < z)

## Features

- Lexical Analysis
- Syntax Analysis
- Semantic Checking
- Abstract Syntax Tree Generation
- Intermediate Code Generation
- Arithmetic Expression Evaluation
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

```--gen_code```: prints MIPS32 assembly code generated from the input source code

```--gen_3ac```: prints the intermediary 3 address code to stdout

The compiler can be run with all, none, or a combination of flags

#### note about linked function println(int):

This compiler has support for printing integers using a function called ```println(int)```. The function definition does not need to be in the source code, as
it is linked to the mips output during the code generation phase. If source code contains calls to ```println(int)``` when the ```--chk_decl``` or ```--print_ast```
flags are on, the compiler will throw an error. Simply add function stub ```int println(int x){}``` to the top of the source code for these flags to work properly.

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
