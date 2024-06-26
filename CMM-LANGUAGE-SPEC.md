# C-- Language Specification
## Lexical Rules
### Tokens

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

### Comments
Comments are a sequence of characters preceeded by ```/*``` and followed by ```*/```

## Syntax Rules
### Grammar Productions
Nonterminals are shown in lowercase.
Terminals are shown in upper-case
`ε` defines an empty sequences
the start symbol of the grammar is _```prog```_

#### prog
- `func_defn prog`
- `var_decl prog`
- `ε`

#### var_decl
- `type id_list SEMI`

#### id_list
- `ID`
- `ID COMMA id_list`

#### type
- `kwINT`

#### func_defn
- `type ID LPAREN opt_formals RPAREN LBRACE opt_var_decls opt_stmt_list RBRACE`

#### opt_formals
- `ε`
- `formals`

#### formals
- `type ID COMMA formals`
- `type ID`

#### opt_var_decls
- `ε`
- `var_decl opt_var_decls`

#### opt_stmt_list
- `stmt opt_stmt_list`
- `ε`

#### stmt
- `fn_call SEMI`
- `while_stmt`
- `if_stmt`
- `assg_stmt`
- `return_stmt`
- `LBRACE opt_stmt_list RBRACE`
- `SEMI`

#### if_stmt
- `kwIF LPAREN bool_exp RPAREN stmt`
- `kwIF LPAREN bool_exp RPAREN stmt kwELSE stmt`

#### while_stmt
- `kwWHILE LPAREN bool_exp RPAREN stmt`

#### return_stmt
- `kwRETURN SEMI`
- `kwRETURN arith_exp SEMI`

#### assg_stmt
- `ID opASSG arith_exp SEMI`

#### fn_call
- `ID LPAREN opt_expr_list RPAREN`

#### opt_expr_list
- `ε`
- `expr_list`

#### expr_list
- `arith_exp COMMA expr_list`
- `arith_exp`

#### arith_exp
- `ID`
- `INTCON`
- `arith_exp arithop arith_exp`
- `LPAREN arith_exp RPAREN`
- `opSUB arith_exp`
- `fn_call`

#### bool_exp
- `arith_exp relop arith_exp`
- `bool_exp logical_op bool_exp`

#### arithop
- `opADD`
- `opSUB`
- `opMUL`
- `opDIV`

#### relop
- `opEQ`
- `opNE`
- `opLE`
- `opLT`
- `opGE`
- `opGT`

#### logical_op
- `opAND`
- `opOR`

## Operator Associativity

### Arithmetic Operators

| Operator                      | Associativity    |
|-------------------------------|------------------|
| opSUB (unary)                 | right to left    |
| opMUL, opDIV                  | left to right    |
| opADD, opSUB (binary)         | left to right    |

### Boolean Operators

| Operator            | Associativity    |
|---------------------|------------------|
| opAND               | right to left    |
| opOR                | left to right    |

# Semantic Rules

## Scopes and Types

There are two scopes in C--. `global` and `local`.

`local` scope pertains to any variable defined within a function, or as a function's formal parameter

`global` scope pertains to any variable defined outside of a function, or a function definition

There are two types in C--, `int` variable and `function`

Thus, these are the combinations of scopes and types:

|            | Global    | Local      |
|------------|-----------|------------|
| Variable   | Yes       | Yes        |
| Function   | Yes       | No         |

