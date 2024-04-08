#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "ast.h"
#include "codegen.h"

int tmp_count = 0;
extern char* cur_id;
extern SymbolTable* scope;

int t_reg = 0;
int fp_offset = 0;
int sp_offset = 0;

Quad* quad_ll = NULL;
Quad* quad_ll_tail = NULL;

void print_global(){
    printf(".align 2\n.data\n_%s: .space 4\n\n", cur_id);
}

void generate_println(){
    printf(".align 2\n.data\n_nl: .asciiz \"\\n\"\n\n.align 2\n.text\n_println:\n");
    printf("  li $v0, 1\n  lw $a0, 0($sp)\n  syscall\n  li $v0, 4\n  la $a0, _nl\n  syscall\n  jr $ra\n");
    printf("main:\n j _main\n\n");
}

void generate_mips(){
    Quad* cur_quad = quad_ll;
    while (cur_quad != NULL){
        switch(cur_quad->op){
            case GC_ENTER:
                print_3ac_comment(cur_quad);
                printf(".align 2\n.text\n_%s:\n", cur_quad->src1->val.st_ref->id);
                printf("  la $sp, -8($sp)\n");
                printf("  sw $fp, 4($sp)\n");
                printf("  sw $ra, 0($sp)\n");
                printf("  la $fp, 0($sp)\n");
                printf("  la $sp, %d($sp)\n", num_st_entries()*-4);
                break;
            
            case GC_ASSG:
                print_3ac_comment(cur_quad);
                Operand* lhs = cur_quad->dest;
                Operand* rhs = cur_quad->src1;
                // local_var = constant
                if (lhs->val.st_ref->is_global == false && rhs->operand_type == OPERAND_INTCONST){
                    
                    printf("  li $t0, %d\n", cur_quad->src1->val.iconst);   // $t0 = constant
                    printf("  sw $t0, %d($fp)\n", cur_quad->dest->val.st_ref->offset); // local_var = $t0
                }
                // local_var = local_var
                else if (lhs->val.st_ref->is_global == false && rhs->operand_type == OPERAND_ST_PTR && rhs->val.st_ref->is_global == false){
                    printf("  lw $t0, %d($fp)\n", cur_quad->src1->val.st_ref->offset);
                    printf("  sw $t0, %d($fp)\n", cur_quad->dest->val.st_ref->offset);
                }
                // local_var = global_var
                else if (lhs->val.st_ref->is_global == false && rhs->val.st_ref->is_global == true){
                    printf("  lw $t0, _%s\n", rhs->val.st_ref->id);
                    printf("  sw $t0, %d($fp)\n", lhs->val.st_ref->offset);
                }
                // global_var = local_var
                else if(lhs->val.st_ref->is_global == true && rhs->val.st_ref->is_global == false){
                    
                    printf("  lw $t0, %d($fp)\n", rhs->val.st_ref->offset);
                    printf("  sw $t0, _%s\n", lhs->val.st_ref->id);
                }
                // global_var = global_var
                else if(lhs->val.st_ref->is_global == true && rhs->val.st_ref->is_global == true){
                    printf("  lw $t0, _%s\n", rhs->val.st_ref->id);
                    printf("  sw $t0, _%s\n", lhs->val.st_ref->id);
                }

                // global_var = constant
                else if(lhs->val.st_ref->is_global == true && rhs->operand_type == OPERAND_INTCONST){
                    printf("  li $t0, %d\n", rhs->val.iconst);
                    printf("  sw $t0, _%s\n", lhs->val.st_ref->id);
                }
                break;

            case GC_PARAM:
                print_3ac_comment(cur_quad);
                if (cur_quad->src1->val.st_ref->is_global == false){
                    printf("  lw $t0, %d($fp)\n", cur_quad->src1->val.st_ref->offset);
                }
                else if (cur_quad->src1->val.st_ref->is_global == true){
                    printf("  lw $t0, _%s\n", cur_quad->src1->val.st_ref->id);
                }
                printf("  la $sp, -4($sp)\n");
                printf("  sw $t0, 0($sp)\n");
                break;

            case GC_CALL:
                print_3ac_comment(cur_quad);
                printf("  jal _%s\n", cur_quad->src1->val.st_ref->id);
                printf("  la $sp, %d($sp)\n", cur_quad->nargs * 4);
                break;

            case GC_RETRIEVE:
                print_3ac_comment(cur_quad);
                break;

            case GC_LEAVE:
                print_3ac_comment(cur_quad);
                break;

            case GC_RETURN_VOID:
                print_3ac_comment(cur_quad);
                printf("  la $sp, 0($fp)\n");
                printf("  lw $ra, 0($sp)\n");
                printf("  lw $fp, 4($sp)\n");
                printf("  la $sp, 8($sp)\n");
                printf("  jr $ra\n");
                break;
            }
        cur_quad = cur_quad->next;
    }
    quad_ll = NULL;
    quad_ll_tail = NULL;
}

int num_st_entries(){
    int count = 0;
    Symbol* cur_sym = scope->head;
    while (cur_sym != NULL){
        count++;
        cur_sym = cur_sym->next;
    }
    return count;
}

void print_3ac_comment(Quad* cur_quad){
    switch(cur_quad->op){
            case GC_ENTER:
                printf("#enter %s\n", cur_quad->src1->val.st_ref->id);
                break;

            case GC_ASSG:
                if (cur_quad->src1->operand_type == OPERAND_ST_PTR){
                    printf("#assign  %s = %s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id);
                }
                else if (cur_quad->src1->operand_type == OPERAND_INTCONST){
                    printf("#assign  %s = %d\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.iconst);
                }
                break;

            case GC_CALL:
                printf("#call %s, %d\n", cur_quad->src1->val.st_ref->id, cur_quad->nargs);
                break; 

            case GC_PARAM:
                printf("#param %s\n", cur_quad->src1->val.st_ref->id);
                break;

            case GC_LEAVE:
                printf("#leave %s\n", cur_quad->src1->val.st_ref->id);
                break;

            case GC_RETURN_VOID:
                printf("#return void\n");
                break;

            case GC_RETRIEVE:
                printf("#retrieve %s, %s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id);
                break;

            default:
                printf("#ERROR IN PRINT_3AC:\n UNKNOWN OP: %d\n", cur_quad->op);
                exit(-1);
                break;
        }

}

void print_3ac(){
    Quad* cur_quad = quad_ll;
    while (cur_quad != NULL){
        switch(cur_quad->op){
            case GC_ENTER:
                printf("enter %s\n", cur_quad->src1->val.st_ref->id);
                break;

            case GC_ASSG:
                if (cur_quad->src1->operand_type == OPERAND_ST_PTR){
                    printf("assign  %s = %s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id);
                }
                else if (cur_quad->src1->operand_type == OPERAND_INTCONST){
                    printf("assign  %s = %d\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.iconst);
                }
                break;

            case GC_CALL:
                printf("call %s, %d\n", cur_quad->src1->val.st_ref->id, cur_quad->nargs);
                break; 

            case GC_PARAM:
                printf("param %s\n", cur_quad->src1->val.st_ref->id);
                break;

            case GC_LEAVE:
                printf("leave %s\n", cur_quad->src1->val.st_ref->id);
                break;

            case GC_RETURN_VOID:
                printf("return void\n\n");
                break;

            case GC_RETRIEVE:
                printf("retrieve %s, %s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id);
                break;

            default:
                printf("ERROR IN PRINT_3AC:\n UNKNOWN OP: %d\n", cur_quad->op);
                exit(-1);
                break;
        }
        cur_quad = cur_quad->next;
    }
}

void create_3ac(ASTnode* cur_node){
    if (cur_node == NULL){
        return;
    }
    switch(cur_node->node_type){
        Quad* this_quad;
        case FUNC_DEF:
            Operand* funcdef_operand = make_operand(OPERAND_ST_PTR, cur_node->st_ref, 0);
            this_quad = newinstr(GC_ENTER, funcdef_operand, NULL, NULL);
            this_quad->nargs = get_num_args(cur_node->st_ref);
            append_quad(this_quad);
            create_3ac(cur_node->child0);
            break;
            
        case STMT_LIST:
            create_3ac(cur_node->child0);
            create_3ac(cur_node->child1);
            break;

        case ASSG:
            // rhs is an integer. must create temp
            if (cur_node->child1->node_type == INTCONST){
                create_3ac(cur_node->child1);
                create_3ac(cur_node->child0);
                Operand* lhs = make_operand(OPERAND_ST_PTR, cur_node->child0->st_ref, 0);
                Operand* rhs = make_operand(OPERAND_ST_PTR, cur_node->child1->st_ref, 0);
                this_quad = newinstr(GC_ASSG, rhs, NULL, lhs);
                append_quad(this_quad);
            }
            // rhs is an identifier, make assg quad
            else if (cur_node->child1->node_type == IDENTIFIER){
                create_3ac(cur_node->child0);
                create_3ac(cur_node->child1);
                Operand* lhs = make_operand(OPERAND_ST_PTR, cur_node->child0->st_ref, 0);
                Operand* rhs = make_operand(OPERAND_ST_PTR, cur_node->child1->st_ref, 0);
                this_quad = newinstr(GC_ASSG, rhs, NULL, lhs);
                append_quad(this_quad);
            }
            break;

        case FUNC_CALL:
            // probably need a gen_params function for this to generate PARAM quads
            create_params_3ac(cur_node->child0); // process parameters (expr_list)
            Operand* func_ref = make_operand(OPERAND_ST_PTR, cur_node->st_ref, 0);
            this_quad = newinstr(GC_CALL, func_ref, NULL, NULL);
            this_quad->nargs = get_num_args(cur_node->st_ref);
            append_quad(this_quad);
            // bogus return value
            Symbol* ret_val_tmp = create_tmp();
            Operand* ret_val = make_operand(OPERAND_ST_PTR, ret_val_tmp, 0);
            this_quad = newinstr(GC_RETRIEVE, func_ref, NULL, ret_val);
            append_quad(this_quad);
            break;

        case EXPR_LIST:
            create_3ac(cur_node->child0);
            create_3ac(cur_node->child1);
            break;

        case INTCONST:
            Symbol* tmp_stref = create_tmp(); // create reference for a temp in the ST
            cur_node->st_ref = tmp_stref; // give the current node a st ref
            Operand* int_operand = make_operand(OPERAND_INTCONST, NULL, cur_node->int_const_val);
            Operand* new_tmp = make_operand(OPERAND_ST_PTR, tmp_stref, 0);
            Quad* new_temp_instr = newinstr(GC_ASSG, int_operand, NULL, new_tmp);
            append_quad(new_temp_instr);
            break;

        case IDENTIFIER:
            break;

        default:
            printf("ERROR IN CREATE_3AC:\n UNKNOWN NODE TYPE: %d\n", cur_node->node_type);
            exit(-1);
            break;
    }
    if (cur_node->node_type == FUNC_DEF){
        Operand* func_ref = make_operand(OPERAND_ST_PTR, cur_node->st_ref, 0);
        Quad* leave_quad = newinstr(GC_LEAVE, func_ref, NULL, NULL);
        append_quad(leave_quad);
        // implicit return
        Operand* implicit_return = make_operand(OPERAND_ST_PTR, cur_node->st_ref, 0);
        Quad* ret_void = newinstr(GC_RETURN_VOID, implicit_return, NULL, NULL);
        append_quad(ret_void);

        int offset = -4;
        Symbol* cur_symbol = scope->head;
        while (cur_symbol != NULL){
            cur_symbol->offset = offset;
            offset -= 4;
            cur_symbol = cur_symbol->next;
        }

        offset = 8;
        int i = 0;
        while (cur_node->arg_list[i+1] != NULL){
            cur_node->arg_list[i+1]->offset = offset;
            offset += 4;
            i++;
        }
        

    }
    return;
}

void create_params_3ac(ASTnode* cur_node){
    if (cur_node == NULL){
        return;
    }
    switch(cur_node->node_type){
        case EXPR_LIST:
            // recurse down to the last expression so the left most param will be on top of the stack
            create_params_3ac(cur_node->child1); // next expression
            create_params_3ac(cur_node->child0); // current expression
            break;
        case IDENTIFIER:
            Operand* id_param = make_operand(OPERAND_ST_PTR, cur_node->st_ref, 0);
            Quad* new_param_instr = newinstr(GC_PARAM, id_param, NULL, NULL);
            append_quad(new_param_instr);
            break;

        case INTCONST:
            // assign the integer to a temp
            Symbol* tmp_stref = create_tmp(); // create reference for a temp in the ST
            cur_node->st_ref = tmp_stref; // give the current node a st ref
            Operand* int_operand = make_operand(OPERAND_INTCONST, NULL, cur_node->int_const_val);
            Operand* new_tmp = make_operand(OPERAND_ST_PTR, tmp_stref, 0);
            Quad* new_temp_assgn = newinstr(GC_ASSG, int_operand, NULL, new_tmp);
            append_quad(new_temp_assgn);
            // create new param quad
            Operand* int_con_param = make_operand(OPERAND_ST_PTR, tmp_stref, 0);
            Quad* int_con_param_instr = newinstr(GC_PARAM, int_con_param, NULL, NULL);
            append_quad(int_con_param_instr);
            break;
    }
    return;
}

Symbol* create_tmp(){
    char* tmp_id = (char*)malloc(10 * sizeof(char));
    sprintf(tmp_id, "tmp%d", tmp_count);
    tmp_count++;
    add_symbol(tmp_id, false);
    scope->head->is_global = false;
    return scope->head;
}

Operand* make_operand(OperandType optype, Symbol* st_ref, int val){
    Operand* new_operand = (Operand*)malloc(sizeof(Operand));
    new_operand->operand_type = optype;
    if (optype == OPERAND_INTCONST){
        new_operand->val.iconst = val;
    }
    else if (optype == OPERAND_ST_PTR){
        new_operand->val.st_ref = st_ref;
    }
    return new_operand;
}

Quad* newinstr(OpType op, Operand* src1, Operand* src2, Operand* dest){
    Quad* new_instr = (Quad*)malloc(sizeof(Quad));
    new_instr->op = op;
    new_instr->src1 = src1;
    new_instr->src2 = src2;
    new_instr->dest = dest;
    return new_instr;
}

void append_quad(Quad* new_quad){
    if (quad_ll == NULL){
        quad_ll = new_quad;
        quad_ll_tail = new_quad;
    }
    else{
        quad_ll_tail->next = new_quad;
        quad_ll_tail = new_quad;
    }
}