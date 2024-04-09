/*
 * File: scanner-driver.c
 * Author: Jonathan Silvestri
 * Purpose: Parser for C--
 */ 
  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "scanner.c"
#include "scanner.h"
#include "parser.h"
#include "ast.h"
#include "codegen.h"

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


int cur_tok;
char* cur_id;
char* last_function_id;
int num_args;


extern char* lexeme;
extern int line_number;
extern int chk_decl_flag;
extern int print_ast_flag;
extern int gen_code_flag;
extern int gen_3ac_flag;
SymbolTable* scope = NULL;
bool in_function;
ASTnode* root = NULL;
extern Quad* quad_ll;
extern Quad* quad_ll_tail;

int parse(){
    cur_tok = get_token();

    if (cur_tok == EOF){
        match(EOF); return 0;
    }
    scope = (SymbolTable*)malloc(sizeof(SymbolTable));
    num_args = 0;

    if (gen_code_flag){
        add_symbol("println", true);
        scope->head->num_args = 1;
    }
    prog();

    if (gen_code_flag){
        generate_println();
    }
    
    return 0;
}

void prog(){
    in_function = false;
    root = NULL;
    if (cur_tok == EOF){
        match(EOF);
        return;
    }
    type();
    cur_id = strdup(lexeme);
    match(ID);
    if (cur_tok == LPAREN){
        add_symbol(cur_id, true);
        root = make_func_ast();
        ASTnode* body = func_defn();
        root->child0 = body;
        if (print_ast_flag) {print_ast(root);}
        if (gen_code_flag){
            create_3ac(root);
            generate_mips();
            quad_ll = NULL;
            quad_ll_tail = NULL;
        }
        if (gen_3ac_flag){
            create_3ac(root);
            print_3ac();
        }
        pop_symbol_table();
        prog();
    }

    // int x, || int x;
    else if (cur_tok == COMMA || cur_tok == SEMI){
        if (gen_code_flag && !in_function){
            print_global();
        }

        add_symbol(cur_id, false);
        var_decl();
        prog();
    }

    else{
        ERRMSG();
    }

}

void var_decl(){
    // int x,
    if (cur_tok == COMMA){
        match(COMMA);
        id_list();
    }
    // int x,y,z || int x;
    match(SEMI);
}


void id_list(){
    cur_id = strdup(lexeme);
    match(ID);
    add_symbol(cur_id, false);

    if (gen_code_flag && !in_function){
        print_global();
    }

    if (cur_tok == COMMA){
        match(COMMA);
        id_list();
    }
}

void type(){
    match(kwINT);
}

ASTnode* func_defn(){
    //type();
    //match(ID);
    
    last_function_id = strdup(cur_id);
    match(LPAREN);
    push_symbol_table();
    opt_formals();
    
    update_arg_num();
    num_args = 0;
    match(RPAREN);
    match(LBRACE);
    opt_var_decls();
    ASTnode* body = opt_stmt_list();
    match(RBRACE);
    return body;
}

void opt_formals(){
    if (cur_tok == kwINT){
        formals();
    }
}

void formals(){
    type();
    cur_id = strdup(lexeme);
    add_symbol(cur_id, false);
    num_args++;

    update_arg_list();
    match(ID);
    if (cur_tok == COMMA){
        match(COMMA);
        formals();
    }
}

void opt_var_decls(){
    if (cur_tok == kwINT){
        type();
        cur_id = strdup(lexeme);
        add_symbol(cur_id, false);
        match(ID);
        var_decl();
        opt_var_decls();
    }
}

ASTnode* opt_stmt_list(){
    ASTnode* body = NULL;
    ASTnode* body_tail = NULL;
    while (cur_tok != RBRACE && cur_tok != EOF){
        ASTnode* stmt_node = stmt();
        if (body == NULL){
            body = make_ast_node(STMT_LIST);
            body->child0 = stmt_node;
            body_tail = body;
        }
        else{
            ASTnode* new_stmt_lst = make_ast_node(STMT_LIST);
            new_stmt_lst->child0 = stmt_node;
            body_tail->child1 = new_stmt_lst;
            body_tail = new_stmt_lst;
        }
    }
    return body;
}

ASTnode* stmt(){
    if (cur_tok == ID){
        cur_id = strdup(lexeme);
        last_function_id = strdup(lexeme);
        match(ID);
        if (cur_tok == LPAREN){
            ASTnode* fn_call_node = fn_call();
            match(SEMI);
            return fn_call_node;
        }
        else if (cur_tok == opASSG){
            ASTnode* lh_node = make_ast_node(IDENTIFIER);
            ASTnode* assg_stmt_root = assg_stmt();
            assg_stmt_root->child0 = lh_node;
            return assg_stmt_root;
        }
        
        else{
            ERRMSG();
        }
        
    }
    else if (cur_tok == kwWHILE){
        return while_stmt();
    }
    else if (cur_tok == kwIF){
        return if_stmt();
    }
    else if (cur_tok == kwRETURN){
        ASTnode* return_node = make_ast_node(RETURN);
        ASTnode* retval_node = return_stmt();
        return_node->child0 = retval_node;
        return return_node;
    }
    else if (cur_tok == LBRACE){
        match(LBRACE);
        ASTnode* opt_stmt_nodes = opt_stmt_list();
        match(RBRACE);
        return opt_stmt_nodes;
    }
    else if (cur_tok == SEMI){
        match(SEMI);
        return NULL;
    }
    else{
        ERRMSG();
    }
}

ASTnode* if_stmt(){
    ASTnode* if_node = make_ast_node(IF);
    match(kwIF);
    match(LPAREN);
    ASTnode* if_condition = bool_exp();
    match(RPAREN);
    ASTnode* if_body = stmt();
    if (cur_tok == kwELSE){
        match(kwELSE);
        ASTnode* if_else = stmt();
        if_node->child2 = if_else;
    }
    if_node->child0 = if_condition;
    if_node->child1 = if_body;
    return if_node;
}

ASTnode* while_stmt(){
    ASTnode* while_node = make_ast_node(WHILE);
    match(kwWHILE);
    match(LPAREN);
    ASTnode* condition_node = bool_exp();
    match(RPAREN);
    ASTnode* while_body = stmt();

    while_node->child0 = condition_node;
    while_node->child1 = while_body;
    return while_node;
}

ASTnode* return_stmt(){
    match(kwRETURN);
    if (cur_tok == SEMI){
        match(SEMI);
        return NULL;
    }
    else if (cur_tok == ID || cur_tok == INTCON){
        ASTnode* retval_node = arith_exp();
        match(SEMI);
        return retval_node;
    }
    else{
        ERRMSG();
    }
}

ASTnode* assg_stmt(){
    //match(ID);
    if (chk_decl_flag){
        if (check_global_scope(cur_id, false) == 0) {SNTXERR();}
    }
    match(opASSG);
    ASTnode* assg_node = make_ast_node(ASSG);
    ASTnode* rh_assg_node = arith_exp();
    match(SEMI);
    
    assg_node->child1 = rh_assg_node;
    return assg_node;
}

ASTnode* fn_call(){
    //match(ID);
    if (chk_decl_flag){
        if (check_global_scope(cur_id, true) == 0 || check_local_scope(cur_id, false) == 1){
            SNTXERR();
        }
    }
    ASTnode* fn_call_node = make_ast_node(FUNC_CALL);
    match(LPAREN);
    num_args = 0;
    ASTnode* fn_call_params = opt_expr_list();

    if (chk_decl_flag){
        check_arg_num();
    }
    num_args = 0;
    match(RPAREN);
    fn_call_node->child0 = fn_call_params;
    return fn_call_node;
}

ASTnode* opt_expr_list(){
    if (cur_tok == ID || cur_tok == INTCON){
        ASTnode* expr_list_hd = expr_list();
        return expr_list_hd;
    }
    return NULL;
}

ASTnode* expr_list(){
    ASTnode* expr_list_hd = make_ast_node(EXPR_LIST);
    ASTnode* expr = arith_exp();
    expr_list_hd->child0 = expr;
    num_args++;
    while (cur_tok == COMMA){
        match(COMMA);
        ASTnode* new_expr_lst = make_ast_node(EXPR_LIST);
        ASTnode* new_expr = arith_exp();
        new_expr_lst->child0 = new_expr;
        if (expr_list_hd->child1 == NULL){
            expr_list_hd->child1 = new_expr_lst;
            expr_list_hd->stmt_lst_tail = new_expr_lst;
        }
        else{
            expr_list_hd->stmt_lst_tail->child1 = new_expr_lst;
            expr_list_hd->stmt_lst_tail = new_expr_lst;
        }
        num_args++;
    }

    return expr_list_hd;
}

ASTnode* bool_exp(){
    ASTnode* lh_node = arith_exp();
    ASTnode* op_node = relop();
    ASTnode* rh_node = arith_exp();
    op_node->child0 = lh_node;
    op_node->child1 = rh_node;
    return op_node;
}

ASTnode* arith_exp(){
    if (cur_tok == ID){
        cur_id = strdup(lexeme);
        if (chk_decl_flag){
            if (check_global_scope(cur_id, false) == 0) {SNTXERR();}
        }

        ASTnode* id_node = make_ast_node(IDENTIFIER);
        match(ID);
        return id_node;
    }
    else if (cur_tok == INTCON){
        ASTnode* int_node = make_ast_node(INTCONST);
        match(INTCON);
        return int_node;
    }
    else{
        ERRMSG();
    }
}
ASTnode* relop(){
    if (cur_tok == opEQ){
        ASTnode* opEQ_node = make_ast_node(EQ);
        match(opEQ);
        return opEQ_node;
    }
    else if (cur_tok == opNE){
        ASTnode* opNE_node = make_ast_node(NE);
        match(opNE);
        return opNE_node;
    }
    else if (cur_tok == opLE){
        ASTnode* opLE_node = make_ast_node(LE);
        match(opLE);
        return opLE_node;
    }
    else if (cur_tok == opLT){
        ASTnode* opLT_node = make_ast_node(LT);
        match(opLT);
        return opLT_node;
    }
    else if (cur_tok == opGE){
        ASTnode* opGE_node = make_ast_node(GE);
        match(opGE);
        return opGE_node;
    }
    else if (cur_tok == opGT){
        ASTnode* opGT_node = make_ast_node(GT);
        match(opGT);
        return opGT_node;
    }
    else{
        ERRMSG();
    }
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
    fprintf(stderr,"PARSE ERROR! (Token: %s | Line %d)\n", token_name[cur_tok], line_number);
    exit(1);
}

void SNTXERR(){
    fprintf(stderr, "SYNTAX ERROR! (Lexeme %s | Line %d)\n", lexeme, line_number);
    fprintf(stderr, "   Current ID: %s\n", cur_id);
    exit(1);
}


int get_num_args(Symbol* function_ref){
    if (function_ref == NULL){
        return -1;
    }
    return function_ref->num_args;
}

void check_arg_num(){
    SymbolTable* cur_scope = scope;
    while (cur_scope != NULL){
        Symbol* cur = cur_scope->head;
        while (cur != NULL){
            // if new_id matches an existing id
            if (cur->is_func && strcmp(cur->id, last_function_id) == 0 && chk_decl_flag){
                if (cur->num_args != num_args) {SNTXERR();}
                return;
            }
            cur = cur->next;
        }
        cur_scope = cur_scope->prev_scope;
    }
}

void update_arg_num(){
    SymbolTable* cur_scope = scope;
    while (cur_scope != NULL){
        Symbol* cur = cur_scope->head;
        while (cur != NULL){
            // if new_id matches an existing id
            if (cur->is_func){
                cur->num_args = num_args;
                return;
            }
            cur = cur->next;
        }
        cur_scope = cur_scope->prev_scope;
    }
}

void push_symbol_table(){
    in_function = true;
    SymbolTable* new_st = (SymbolTable*)malloc(sizeof(SymbolTable));
    new_st->prev_scope = scope;
    new_st->head = NULL;
    scope = new_st;
}

void pop_symbol_table(){
    in_function = false;
    SymbolTable* to_free = scope;
    scope = scope->prev_scope;
    free(to_free);
}

int check_local_scope(char* new_id, bool is_function){
    Symbol* cur = scope->head;
    while (cur != NULL){
        // if new_id matches an existing id
        if (strcmp(cur->id, new_id) == 0){
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}

int check_global_scope(char* new_id, bool is_function){
    SymbolTable* cur_scope = scope;
    while (cur_scope != NULL){
        Symbol* cur = cur_scope->head;
        while (cur != NULL){
            // if new_id matches an existing id
            if (strcmp(cur->id, new_id) == 0 && cur->is_func == is_function){
                return 1;
            }
            cur = cur->next;
        }
        cur_scope = cur_scope->prev_scope;
    }
    return 0;
}

void add_symbol(char* new_id, bool is_function){
    if (check_local_scope(new_id, is_function) == 1 && chk_decl_flag){   // duplicate ID
        SNTXERR();
    }
    Symbol* new_symbol = (Symbol*)malloc(sizeof(Symbol));
    new_symbol->id = strdup(new_id);
    new_symbol->num_args = 0;
    new_symbol->is_func = is_function;
    new_symbol->next = scope->head;
    new_symbol->is_global = !(in_function);

    scope->head = new_symbol;
}

ASTnode* make_ast_node(NodeType node_type){
    ASTnode* node = (ASTnode*)malloc(sizeof(ASTnode));
    node->node_type = node_type;
    if (node_type == INTCONST){
        node->int_const_val = atoi(lexeme);
        return node;
    }

    if (node_type == IDENTIFIER || node_type == FUNC_CALL){
        bool is_func;
        if (node_type == IDENTIFIER){is_func = false;}
        if (node_type == FUNC_CALL){is_func = true;}
        SymbolTable* cur_scope = scope;
        while (cur_scope != NULL){
            Symbol* cur = cur_scope->head;
            while (cur != NULL){
                if (strcmp(cur->id, cur_id) == 0 && cur->is_func == is_func){
                    node->st_ref = cur;
                    return node;
                }
                cur = cur->next;  
            }
            cur_scope = cur_scope->prev_scope;
        }
    }
    return node;
}

ASTnode* make_func_ast(){
    ASTnode* node = (ASTnode*)malloc(sizeof(ASTnode));
    node->node_type = FUNC_DEF;
    node->st_ref = scope->head;
    return node;
}

void update_arg_list(){
    root->arg_list[num_args] = scope->head;
}

/* 
 * ptr: an arbitrary non-NULL AST pointer; ast_node_type() returns the node type 
 * for the AST node ptr points to.
 */
NodeType ast_node_type(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    //printf("%d\n", node->node_type);
    return node->node_type;
}

/* 
 * ptr: pointer to an AST for a function definition; func_def_name() returns 
 * a pointer to the function name (a string) of the function definition AST that
 * ptr points to.
 */
char * func_def_name(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->st_ref->id;
}

/*
 * ptr: pointer to an AST for a function definition; func_def_nargs() returns 
 * the number of formal parameters for that function.
 */
int func_def_nargs(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->st_ref->num_args;
}

/*
 * ptr: pointer to an AST for a function definition, n is an integer. If n > 0
 * and n <= no. of arguments for the function, then func_def_argname() returns 
 * a pointer to the name (a string) of the nth formal parameter for that function;
 * the first formal parameter corresponds to n == 1.  If the value of n is outside
 * these parameters, the behavior of this function is undefined.
 */
char *func_def_argname(void *ptr, int n){
    if (n > 0 || n <= func_def_nargs(ptr)) {
        ASTnode* node = (ASTnode*)ptr;
        return node->arg_list[n]->id;
    }
    return "";
    
}

/* 
 * ptr: pointer to an AST for a function definition; func_def_body() returns 
 * a pointer to the AST that is the function body of the function that ptr
 * points to.
 */
void * func_def_body(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child0;
}

/*
 * ptr: pointer to an AST node for a function call; func_call_callee() returns 
 * a pointer to a string that is the name of the function being called.
 */
char * func_call_callee(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->st_ref->id;
}

/*
 * ptr: pointer to an AST node for a function call; func_call_args() returns 
 * a pointer to the AST that is the list of arguments to the call.
 */
void * func_call_args(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child0;
}

/*
 * ptr: pointer to an AST node for a statement list; stmt_list_head() returns 
 * a pointer to the AST of the statement at the beginning of this list.
 */
void * stmt_list_head(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child0;
}

/*
 * ptr: pointer to an AST node for a statement list; stmt_list_rest() returns 
 * a pointer to the AST of the rest of this list (i.e., the pointer to the
 * next node in the list).
 */
void * stmt_list_rest(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child1;
}

/*
 * ptr: pointer to an AST node for an expression list; expr_list_head() returns 
 * a pointer to the AST of the expression at the beginning of this list.
 */
void * expr_list_head(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child0;
}

/*
 * ptr: pointer to an AST node for an expression list; expr_list_rest() returns 
 * a pointer to the AST of the rest of this list (i.e., the pointer to the
 * next node in the list).
 */
void * expr_list_rest(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child1;
}

/*
 * ptr: pointer to an AST node for an IDENTIFIER; expr_id_name() returns a 
 * pointer to the name of the identifier (a string).
 */
char *expr_id_name(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->st_ref->id;
}

/*
 * ptr: pointer to an AST node for an INTCONST; expr_intconst_val() returns the
 * integer value of the constant.
 */
int expr_intconst_val(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->int_const_val;
}

/*
 * ptr: pointer to an AST node for an arithmetic or boolean expression.
 * expr_operand_1() returns a pointer to the AST of the first operand.
 */
void * expr_operand_1(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child0;
}

/*
 * ptr: pointer to an AST node for an arithmetic or boolean expression.
 * expr_operand_2() returns a pointer to the AST of the second operand.
 */
void * expr_operand_2(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child1;
}

/*
 * ptr: pointer to an AST node for an IF statement.  stmt_if_expr() returns
 * a pointer to the AST for the expression tested by the if statement.
 */
void * stmt_if_expr(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child0;
}

/*
 * ptr: pointer to an AST node for an IF statement.  stmt_if_then() returns
 * a pointer to the AST for the then-part of the if statement, i.e., the
 * statement to be executed if the condition is true.
 */
void * stmt_if_then(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child1;
}

/*
 * ptr: pointer to an AST node for an IF statement.  stmt_if_else() returns
 * a pointer to the AST for the else-part of the if statement, i.e., the
 * statement to be executed if the condition is false.
 */
void * stmt_if_else(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child2;
}

/*
 * ptr: pointer to an AST node for an assignment statement.  stmt_assg_lhs()
 * returns a pointer to the name of the identifier on the LHS of the
 * assignment.
 */
char *stmt_assg_lhs(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child0->st_ref->id;
}

/*
 * ptr: pointer to an AST node for an assignment statement.  stmt_assg_rhs()
 * returns a pointer to the AST of the expression on the RHS of the assignment.
 */
void *stmt_assg_rhs(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child1;
}

/*
 * ptr: pointer to an AST node for a while statement.  stmt_while_expr()
 * returns a pointer to the AST of the expression tested by the while statement.
 */
void *stmt_while_expr(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child0;
}

/*
 * ptr: pointer to an AST node for a while statement.  stmt_while_body()
 * returns a pointer to the AST of the body of the while statement.
 */
void *stmt_while_body(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child1;
}

/*
 * ptr: pointer to an AST node for a return statement.  stmt_return_expr()
 * returns a pointer to the AST of the expression whose value is returned.
 */
void *stmt_return_expr(void *ptr){
    ASTnode* node = (ASTnode*)ptr;
    return node->child0;
}
