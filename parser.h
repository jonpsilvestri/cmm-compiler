#include "ast.h"
#include "scanner.h"

#ifndef __PARSER_H__
#define __PARSER_H__
 
int parse();
void prog();
ASTnode* func_defn();
void type();
void match(Token);
void ERRMSG();
void opt_formals();
void opt_var_decls();
ASTnode* opt_stmt_list();
ASTnode* opt_expr_list();
ASTnode* fn_call();
ASTnode* stmt();
void var_decl();
void id_list();
void formals();
ASTnode* arith_exp();
ASTnode* bool_exp();
ASTnode* expr_list();
ASTnode* assg_stmt();
ASTnode* return_stmt();
ASTnode* while_stmt();
ASTnode* if_stmt();
ASTnode* relop();
ASTnode* logical_op();
ASTnode* arith_exp_rest();
ASTnode* expr1();
ASTnode* expr1_rest();
ASTnode* expr2();
ASTnode* expr3();
ASTnode* bool1();
ASTnode* bool_exp_rest(ASTnode*);
ASTnode* bool1_rest(ASTnode*);
ASTnode* term();


void SNTXERR();



typedef struct SymbolTable {
	struct SymbolTable* prev_scope;
	struct Symbol* head;
} SymbolTable;

typedef struct Symbol {
	char* id;
	int num_args;
	bool is_func;
	struct Symbol* next;
	bool is_global;
	int offset;
} Symbol;

void add_symbol(char*, bool);
int check_local_scope(char*, bool);
int check_global_scope(char*, bool);
void push_symbol_table();
void pop_symbol_table();
void update_arg_num();
void check_arg_num(ASTnode*, char*);
int get_num_args(Symbol*);



#endif  /* __PARSER_H__ */
