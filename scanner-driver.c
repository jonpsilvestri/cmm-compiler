/*
 * File: scanner-driver.c
 * Author: Jonathan Silvestri
 * Purpose: The driver code for the scanner
 */

#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"

/*
 * token_name is an array of strings that gives, for each token value,
 * a string indicating the type of token.
 */
char* token_name[] = {
  "UNDEF",
  "ID",
  "INTCON",
  "LPAREN",
  "RPAREN",
  "LBRACE",
  "RBRACE",
  "COMMA",
  "SEMI",
  "kwINT",
  "kwIF",
  "kwELSE",
  "kwWHILE",
  "kwRETURN",
  "opASSG",
  "opADD",
  "opSUB",
  "opMUL",
  "opDIV",
  "opEQ",
  "opNE",
  "opGT",
  "opGE",
  "opLT",
  "opLE",
  "opAND",
  "opOR",
  "opNOT",
};

extern int get_token();
extern char* lexeme;

int main() {
  int tok;

  while ((tok = get_token()) != EOF) {
    print_token(tok, lexeme);
  }

  return 0;
}

void print_token(Token tok, char* lexeme) {
  if (tok < UNDEF || tok > opNOT) {
    printf("TOKEN VALUE OUT OF BOUNDS: %d\n", tok);
  }
  else {
    printf("%s : %s\n", token_name[tok], lexeme);
  }
}
