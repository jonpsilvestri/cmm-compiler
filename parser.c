/*
 * File: scanner-driver.c
 * Author: Jonathan Silvestri
 * Purpose: Parser for C-- language
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.c"
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


int parse();
void prog();
void func_defn();
void type();
void match(Token);
void ERRMSG();
void opt_formals();
void opt_var_decls();
void opt_stmt_list();
void opt_expr_list();
void fn_call();
void stmt();


int cur_tok;
extern int line_number;


int parse(){
    cur_tok = get_token();
    prog();
    return 0;
}

void prog(){
    func_defn();

}

void type(){
    match(kwINT);
}

void func_defn(){
    type();
    match(ID);
    match(LPAREN);
    opt_formals();
    match(RPAREN);
    match(LBRACE);
    opt_var_decls();
    opt_stmt_list();
    match(RBRACE);
}

void opt_formals(){
    return;
}

void opt_var_decls(){
    return;
}

void opt_stmt_list(){
    while (cur_tok != RBRACE && cur_tok != EOF){
        stmt();
    }
}

void stmt(){
    fn_call();
    match(SEMI);
}

void fn_call(){
    match(ID);
    match(LPAREN);
    opt_expr_list();
    match(RPAREN);
}

void opt_expr_list(){
    return;
}

void match(Token expected) {
    if (cur_tok == expected) {
        cur_tok = get_token();
    }   
    else {
        ERRMSG();
    }
}

void ERRMSG(){
    printf("Error on line %d\n", line_number);
    exit(1);
}