#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
 
#include "ast.h"
#include "codegen.h"

int tmp_count = 0;
extern char* cur_id;
extern SymbolTable* scope;

int nested_depth = 1;
int label_count = 0;

int while_depth = 1;
int while_count = 0;
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
            
            case GC_ADD:
                print_3ac_comment(cur_quad);
                // local_var + local_var
                if (cur_quad->src1->val.st_ref->is_global == false && cur_quad->src2->val.st_ref->is_global == false){
                    printf("  lw $t0, %d($fp)\n", cur_quad->src1->val.st_ref->offset);
                    printf("  lw $t1, %d($fp)\n", cur_quad->src2->val.st_ref->offset);
                }
                // global_var + local_var
                else if (cur_quad->src1->val.st_ref->is_global == true && cur_quad->src2->val.st_ref->is_global == false){
                    printf("  lw $t0, _%s\n", cur_quad->src1->val.st_ref->id);
                    printf("  lw $t1, %d($fp)\n", cur_quad->src2->val.st_ref->offset);
                }
                // local_var + global_var
                else if (cur_quad->src1->val.st_ref->is_global == false && cur_quad->src2->val.st_ref->is_global == true){
                    printf("  lw $t0, %d($fp)\n", cur_quad->src1->val.st_ref->offset);
                    printf("  lw $t1, _%s\n", cur_quad->src2->val.st_ref->id);
                }
                // global_var + global_var
                else if (cur_quad->src1->val.st_ref->is_global == true && cur_quad->src2->val.st_ref->is_global == true){
                    printf("  lw $t0, _%s\n", cur_quad->src1->val.st_ref->id);
                    printf("  lw $t1, _%s\n", cur_quad->src2->val.st_ref->id);
                }
                
                // do addition
                printf("  add $t0, $t0, $t1\n");

                // dst is local
                if (cur_quad->dest->val.st_ref->is_global == false)
                    printf("  sw $t0, %d($fp)\n", cur_quad->dest->val.st_ref->offset);
                // dst is global
                else if (cur_quad->dest->val.st_ref->is_global == true)
                    printf("  sw $t0, _%s\n", cur_quad->dest->val.st_ref->id);
                break;

            case GC_SUB:
                print_3ac_comment(cur_quad);
                // local_var - local_var
                if (cur_quad->src1->val.st_ref->is_global == false && cur_quad->src2->val.st_ref->is_global == false){
                    printf("  lw $t0, %d($fp)\n", cur_quad->src1->val.st_ref->offset);
                    printf("  lw $t1, %d($fp)\n", cur_quad->src2->val.st_ref->offset);
                }
                // global_var - local_var
                else if (cur_quad->src1->val.st_ref->is_global == true && cur_quad->src2->val.st_ref->is_global == false){
                    printf("  lw $t0, _%s\n", cur_quad->src1->val.st_ref->id);
                    printf("  lw $t1, %d($fp)\n", cur_quad->src2->val.st_ref->offset);
                }
                // local_var - global_var
                else if (cur_quad->src1->val.st_ref->is_global == false && cur_quad->src2->val.st_ref->is_global == true){
                    printf("  lw $t0, %d($fp)\n", cur_quad->src1->val.st_ref->offset);
                    printf("  lw $t1, _%s\n", cur_quad->src2->val.st_ref->id);
                }
                // global_var - global_var
                else if (cur_quad->src1->val.st_ref->is_global == true && cur_quad->src2->val.st_ref->is_global == true){
                    printf("  lw $t0, _%s\n", cur_quad->src1->val.st_ref->id);
                    printf("  lw $t1, _%s\n", cur_quad->src2->val.st_ref->id);
                }
                
                // do subtraction
                printf("  sub $t0, $t0, $t1\n");

                // dst is local
                if (cur_quad->dest->val.st_ref->is_global == false)
                    printf("  sw $t0, %d($fp)\n", cur_quad->dest->val.st_ref->offset);
                // dst is global
                else if (cur_quad->dest->val.st_ref->is_global == true)
                    printf("  sw $t0, _%s\n", cur_quad->dest->val.st_ref->id);
                break;

            case GC_MULT:
                print_3ac_comment(cur_quad);
                // local_var * local_var
                if (cur_quad->src1->val.st_ref->is_global == false && cur_quad->src2->val.st_ref->is_global == false){
                    printf("  lw $t0, %d($fp)\n", cur_quad->src1->val.st_ref->offset);
                    printf("  lw $t1, %d($fp)\n", cur_quad->src2->val.st_ref->offset);
                }
                // global_var * local_var
                else if (cur_quad->src1->val.st_ref->is_global == true && cur_quad->src2->val.st_ref->is_global == false){
                    printf("  lw $t0, _%s\n", cur_quad->src1->val.st_ref->id);
                    printf("  lw $t1, %d($fp)\n", cur_quad->src2->val.st_ref->offset);
                }
                // local_var * global_var
                else if (cur_quad->src1->val.st_ref->is_global == false && cur_quad->src2->val.st_ref->is_global == true){
                    printf("  lw $t0, %d($fp)\n", cur_quad->src1->val.st_ref->offset);
                    printf("  lw $t1, _%s\n", cur_quad->src2->val.st_ref->id);
                }
                // global_var * global_var
                else if (cur_quad->src1->val.st_ref->is_global == true && cur_quad->src2->val.st_ref->is_global == true){
                    printf("  lw $t0, _%s\n", cur_quad->src1->val.st_ref->id);
                    printf("  lw $t1, _%s\n", cur_quad->src2->val.st_ref->id);
                }
                
                // do multiplication
                printf("  mul $t0, $t0, $t1\n");

                // dst is local
                if (cur_quad->dest->val.st_ref->is_global == false)
                    printf("  sw $t0, %d($fp)\n", cur_quad->dest->val.st_ref->offset);
                // dst is global
                else if (cur_quad->dest->val.st_ref->is_global == true)
                    printf("  sw $t0, _%s\n", cur_quad->dest->val.st_ref->id);
                break;

            case GC_DIV:
                print_3ac_comment(cur_quad);
                // local_var / local_var
                if (cur_quad->src1->val.st_ref->is_global == false && cur_quad->src2->val.st_ref->is_global == false){
                    printf("  lw $t0, %d($fp)\n", cur_quad->src1->val.st_ref->offset);
                    printf("  lw $t1, %d($fp)\n", cur_quad->src2->val.st_ref->offset);
                }
                // global_var / local_var
                else if (cur_quad->src1->val.st_ref->is_global == true && cur_quad->src2->val.st_ref->is_global == false){
                    printf("  lw $t0, _%s\n", cur_quad->src1->val.st_ref->id);
                    printf("  lw $t1, %d($fp)\n", cur_quad->src2->val.st_ref->offset);
                }
                // local_var / global_var
                else if (cur_quad->src1->val.st_ref->is_global == false && cur_quad->src2->val.st_ref->is_global == true){
                    printf("  lw $t0, %d($fp)\n", cur_quad->src1->val.st_ref->offset);
                    printf("  lw $t1, _%s\n", cur_quad->src2->val.st_ref->id);
                }
                // global_var / global_var
                else if (cur_quad->src1->val.st_ref->is_global == true && cur_quad->src2->val.st_ref->is_global == true){
                    printf("  lw $t0, _%s\n", cur_quad->src1->val.st_ref->id);
                    printf("  lw $t1, _%s\n", cur_quad->src2->val.st_ref->id);
                }
                
                // do division
                printf("  div $t0, $t0, $t1\n");

                // dst is local
                if (cur_quad->dest->val.st_ref->is_global == false)
                    printf("  sw $t0, %d($fp)\n", cur_quad->dest->val.st_ref->offset);
                // dst is global
                else if (cur_quad->dest->val.st_ref->is_global == true)
                    printf("  sw $t0, _%s\n", cur_quad->dest->val.st_ref->id);
                break;

            case GC_UMINUS:
                print_3ac_comment(cur_quad);
                // src is local
                if (cur_quad->src1->val.st_ref->is_global == false)
                    printf("  lw $t0, %d($fp)\n", cur_quad->src1->val.st_ref->offset);

                // src is global
                else if (cur_quad->src1->val.st_ref->is_global == true)
                    printf("  lw $t0, _%s\n", cur_quad->src1->val.st_ref->id);
                
                // do unary minus
                printf("  neg $t0, $t0\n");

                // dst is local
                if (cur_quad->dest->val.st_ref->is_global == false)
                    printf("  sw $t0, %d($fp)\n", cur_quad->dest->val.st_ref->offset);

                // dst is global
                else if (cur_quad->dest->val.st_ref->is_global == true)
                    printf("  sw $t0, _%s\n", cur_quad->dest->val.st_ref->id);
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

            case GC_GT:
                print_3ac_comment(cur_quad);
                Symbol* lhs_gt = cur_quad->src1->val.st_ref;
                if (lhs_gt->is_global) {printf("  lw $t0, _%s\n", lhs_gt->id);}
                else {printf("  lw $t0, %d($fp)\n", lhs_gt->offset);}

                Symbol* rhs_gt = cur_quad->src2->val.st_ref;
                if (rhs_gt->is_global) {printf("  lw $t1, _%s\n", rhs_gt->id);}
                else{printf("  lw $t1, %d($fp)\n", rhs_gt->offset);}
                
                printf("  bgt $t0, $t1, _%s\n", cur_quad->label);
                break;

            case GC_EQ:
                print_3ac_comment(cur_quad);
                Symbol* lhs_eq = cur_quad->src1->val.st_ref;
                if (lhs_eq->is_global) {printf("  lw $t0, _%s\n", lhs_eq->id);}
                else {printf("  lw $t0, %d($fp)\n", lhs_eq->offset);}

                Symbol* rhs_eq = cur_quad->src2->val.st_ref;
                if (rhs_eq->is_global) {printf("  lw $t1, _%s\n", rhs_eq->id);}
                else{printf("  lw $t1, %d($fp)\n", rhs_eq->offset);}
                printf("  beq $t0, $t1, _%s\n", cur_quad->label);
                break;

            case GC_NE:
                print_3ac_comment(cur_quad);
                Symbol* lhs_ne = cur_quad->src1->val.st_ref;
                if (lhs_ne->is_global) {printf("  lw $t0, _%s\n", lhs_ne->id);}
                else {printf("  lw $t0, %d($fp)\n", lhs_ne->offset);}

                Symbol* rhs_ne = cur_quad->src2->val.st_ref;
                if (rhs_ne->is_global) {printf("  lw $t1, _%s\n", rhs_ne->id);}
                else{printf("  lw $t1, %d($fp)\n", rhs_ne->offset);}
                printf("  bne $t0, $t1, _%s\n", cur_quad->label);
                break;

            case GC_GE:
                print_3ac_comment(cur_quad);
                Symbol* lhs_ge = cur_quad->src1->val.st_ref;
                if (lhs_ge->is_global) {printf("  lw $t0, _%s\n", lhs_ge->id);}
                else {printf("  lw $t0, %d($fp)\n", lhs_ge->offset);}

                Symbol* rhs_ge = cur_quad->src2->val.st_ref;
                if (rhs_ge->is_global) {printf("  lw $t1, _%s\n", rhs_ge->id);}
                else{printf("  lw $t1, %d($fp)\n", rhs_ge->offset);}
                printf("  bge $t0, $t1, _%s\n", cur_quad->label);
                break;

            case GC_LE:
                print_3ac_comment(cur_quad);
                Symbol* lhs_le = cur_quad->src1->val.st_ref;
                if (lhs_le->is_global) {printf("  lw $t0, _%s\n", lhs_le->id);}
                else {printf("  lw $t0, %d($fp)\n", lhs_le->offset);}

                Symbol* rhs_le = cur_quad->src2->val.st_ref;
                if (rhs_le->is_global) {printf("  lw $t1, _%s\n", rhs_le->id);}
                else{printf("  lw $t1, %d($fp)\n", rhs_le->offset);}
                printf("  ble $t0, $t1, _%s\n", cur_quad->label);
                break;

            case GC_LT:
                print_3ac_comment(cur_quad);
                Symbol* lhs_lt = cur_quad->src1->val.st_ref;
                if (lhs_lt->is_global) {printf("  lw $t0, _%s\n", lhs_lt->id);}
                else {printf("  lw $t0, %d($fp)\n", lhs_lt->offset);}

                Symbol* rhs_lt = cur_quad->src2->val.st_ref;
                if (rhs_lt->is_global) {printf("  lw $t1, _%s\n", rhs_lt->id);}
                else{printf("  lw $t1, %d($fp)\n", rhs_lt->offset);}

                printf("  blt $t0, $t1, _%s\n", cur_quad->label);
                break;

            case GC_GOTO:
                print_3ac_comment(cur_quad);
                printf("  j _%s\n", cur_quad->label);
                break;

            case GC_LABEL:
                print_3ac_comment(cur_quad);
                printf("_%s:\n", cur_quad->label);
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
                printf("  sw $v0, %d($fp)\n", cur_quad->dest->val.st_ref->offset);
                break;

            case GC_LEAVE:
                print_3ac_comment(cur_quad);
                break;

            case GC_RETURN:
                print_3ac_comment(cur_quad);
                if (cur_quad->src1->val.st_ref->is_global == false)
                    printf("  lw $t0, %d($fp)\n", cur_quad->src1->val.st_ref->offset);
                else if (cur_quad->src1->val.st_ref->is_global == true)
                    printf("  lw $t0, _%s\n", cur_quad->src1->val.st_ref->id);
                printf("  add $v0, $zero, $t0\n");
                printf("  la $sp, 0($fp)\n");
                printf("  lw $ra, 0($sp)\n");
                printf("  lw $fp, 4($sp)\n");
                printf("  la $sp, 8($sp)\n");
                printf("  jr $ra\n");
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

            case GC_LABEL:
                printf("#label %s\n", cur_quad->label);
                break;

            case GC_GT:
                printf("#if %s > %s, goto %s\n", cur_quad->src1->val.st_ref->id, cur_quad->src2->val.st_ref->id, cur_quad->label);
                break;

            case GC_EQ:
                printf("#if %s == %s, goto %s\n", cur_quad->src1->val.st_ref->id, cur_quad->src2->val.st_ref->id, cur_quad->label);
                break;

            case GC_LT:
                printf("#if %s < %s, goto %s\n", cur_quad->src1->val.st_ref->id, cur_quad->src2->val.st_ref->id, cur_quad->label);
                break;

            case GC_NE:
                printf("#if %s != %s, goto %s\n", cur_quad->src1->val.st_ref->id, cur_quad->src2->val.st_ref->id, cur_quad->label);
                break;

            case GC_LE:
                printf("#if %s <= %s, goto %s\n", cur_quad->src1->val.st_ref->id, cur_quad->src2->val.st_ref->id, cur_quad->label);
                break;

            case GC_GE:
                printf("#if %s >= %s, goto %s\n", cur_quad->src1->val.st_ref->id, cur_quad->src2->val.st_ref->id, cur_quad->label);
                break;

            case GC_GOTO:
                printf("#goto %s\n", cur_quad->label);
                break;

            case GC_ADD:
                printf("#add %s = %s + %s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id, cur_quad->src2->val.st_ref->id);
                break;

            case GC_SUB:
                printf("#sub %s = %s - %s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id, cur_quad->src2->val.st_ref->id);
                break;

            case GC_MULT:
                printf("#mul %s = %s * %s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id, cur_quad->src2->val.st_ref->id);
                break;

            case GC_DIV:
                printf("#div %s = %s / %s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id, cur_quad->src2->val.st_ref->id);
                break;

            case GC_UMINUS:
                printf("#uminus %s = -%s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id);
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
                printf("#retrieve %s\n", cur_quad->dest->val.st_ref->id);
                break;

            case GC_RETURN:
                printf("#return %s\n", cur_quad->src1->val.st_ref->id);
                break;

            default:
                printf("#ERROR IN PRINT_3AC COMMENT:\n UNKNOWN OP: %d\n", cur_quad->op);
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

            case GC_LABEL:
                printf("label %s\n", cur_quad->label);
                break;

            case GC_GOTO:
                printf("goto %s\n", cur_quad->label);
                break;

            case GC_LE:
                Operand* le_lhs = cur_quad->src1;
                Operand* le_rhs = cur_quad->src2;
                printf("if %s <= %s, goto %s\n", le_lhs->val.st_ref->id, le_rhs->val.st_ref->id, cur_quad->label);
                break;

            case GC_NE:
                Operand* ne_lhs = cur_quad->src1;
                Operand* ne_rhs = cur_quad->src2;
                printf("if %s != %s, goto %s\n", ne_lhs->val.st_ref->id, ne_rhs->val.st_ref->id, cur_quad->label);
                break;

            case GC_GE:
                Operand* ge_lhs = cur_quad->src1;
                Operand* ge_rhs = cur_quad->src2;
                printf("if %s >= %s, goto %s\n", ge_lhs->val.st_ref->id, ge_rhs->val.st_ref->id, cur_quad->label);
                break;

            case GC_GT:
                Operand* gt_lhs = cur_quad->src1;
                Operand* gt_rhs = cur_quad->src2;
                printf("if %s > %s, goto %s\n", gt_lhs->val.st_ref->id, gt_rhs->val.st_ref->id, cur_quad->label);
                break;  

            case GC_EQ:
                Operand* eq_lhs = cur_quad->src1;
                Operand* eq_rhs = cur_quad->src2;
                printf("if %s == %s, goto %s\n", eq_lhs->val.st_ref->id, eq_rhs->val.st_ref->id, cur_quad->label);
                break;

            case GC_LT:
                Operand* lt_lhs = cur_quad->src1;
                Operand* lt_rhs = cur_quad->src2;
                printf("if %s < %s, goto %s\n", lt_lhs->val.st_ref->id, lt_rhs->val.st_ref->id, cur_quad->label);
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

            case GC_RETURN:
                printf("return %s\n", cur_quad->src1->val.st_ref->id);
                break;

            case GC_RETURN_VOID:
                printf("return void\n\n");
                break;

            case GC_RETRIEVE:
                printf("retrieve %s, %s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id);
                break;

            case GC_ADD:
                printf("add %s, %s, %s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id, cur_quad->src2->val.st_ref->id);
                break;

            case GC_SUB:
                printf("sub %s, %s, %s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id, cur_quad->src2->val.st_ref->id);
                break;

            case GC_MULT:
                printf("mul %s, %s, %s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id, cur_quad->src2->val.st_ref->id);
                break;

            case GC_DIV:
                printf("div %s, %s, %s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id, cur_quad->src2->val.st_ref->id);
                break;

            case GC_UMINUS:
                printf("uminus -%s, %s\n", cur_quad->dest->val.st_ref->id, cur_quad->src1->val.st_ref->id);
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

            else if (cur_node->child1->node_type == FUNC_CALL){
                create_3ac(cur_node->child1); //process function call
                create_3ac(cur_node->child0); // process identifier

                Operand* lhs = make_operand(OPERAND_ST_PTR, cur_node->child0->st_ref, 0);
                Operand* rhs = make_operand(OPERAND_ST_PTR, cur_node->child1->st_ref, 0);
                this_quad = newinstr(GC_ASSG, rhs, NULL, lhs);
                append_quad(this_quad);
            }        

            else {
                create_3ac(cur_node->child0);
                create_3ac(cur_node->child1);
                Operand* lhs = make_operand(OPERAND_ST_PTR, cur_node->child0->st_ref, 0);
                Operand* rhs = make_operand(OPERAND_ST_PTR, cur_node->child1->st_ref, 0);
                this_quad = newinstr(GC_ASSG, rhs, NULL, lhs);
                append_quad(this_quad);

            }    
            break;

        case UMINUS:
            create_3ac(cur_node->child0);
            Symbol* uminus_tmp = create_tmp();
            cur_node->st_ref = uminus_tmp;
            Operand* uminus_dst = make_operand(OPERAND_ST_PTR, uminus_tmp, 0);
            Operand* uminus_src = make_operand(OPERAND_ST_PTR, cur_node->child0->st_ref, 0);
            this_quad = newinstr(GC_UMINUS, uminus_src, NULL, uminus_dst);
            append_quad(this_quad);
            break;

        case MUL:
            create_3ac(cur_node->child0);
            create_3ac(cur_node->child1);
            Symbol* mul_dest = create_tmp();
            Operand* mul_dest_operand = make_operand(OPERAND_ST_PTR, mul_dest, 0);
            cur_node->st_ref = mul_dest;
            Operand* mul_lhs = make_operand(OPERAND_ST_PTR, cur_node->child0->st_ref, 0);
            Operand* mul_rhs = make_operand(OPERAND_ST_PTR, cur_node->child1->st_ref, 0);
            this_quad = newinstr(GC_MULT, mul_lhs, mul_rhs, mul_dest_operand);
            append_quad(this_quad);
            break;

        case DIV:
            create_3ac(cur_node->child0);
            create_3ac(cur_node->child1);
            Symbol* div_dest = create_tmp();
            Operand* div_dest_operand = make_operand(OPERAND_ST_PTR, div_dest, 0);
            cur_node->st_ref = div_dest;
            Operand* div_lhs = make_operand(OPERAND_ST_PTR, cur_node->child0->st_ref, 0);
            Operand* div_rhs = make_operand(OPERAND_ST_PTR, cur_node->child1->st_ref, 0);
            this_quad = newinstr(GC_DIV, div_lhs, div_rhs, div_dest_operand);
            append_quad(this_quad);
            break;

        case ADD:
            create_3ac(cur_node->child0);
            create_3ac(cur_node->child1);
            Symbol* add_dest = create_tmp();
            Operand* add_dest_operand = make_operand(OPERAND_ST_PTR, add_dest, 0);
            cur_node->st_ref = add_dest;
            Operand* add_lhs = make_operand(OPERAND_ST_PTR, cur_node->child0->st_ref, 0);
            Operand* add_rhs = make_operand(OPERAND_ST_PTR, cur_node->child1->st_ref, 0);
            this_quad = newinstr(GC_ADD, add_lhs, add_rhs, add_dest_operand);
            append_quad(this_quad);
            break;

        case SUB:
            create_3ac(cur_node->child0);
            create_3ac(cur_node->child1);
            Symbol* sub_dest = create_tmp();
            Operand* sub_dest_operand = make_operand(OPERAND_ST_PTR, sub_dest, 0);
            cur_node->st_ref = sub_dest;
            Operand* sub_lhs = make_operand(OPERAND_ST_PTR, cur_node->child0->st_ref, 0);
            Operand* sub_rhs = make_operand(OPERAND_ST_PTR, cur_node->child1->st_ref, 0);
            this_quad = newinstr(GC_SUB, sub_lhs, sub_rhs, sub_dest_operand);
            append_quad(this_quad);
            break;

        case FUNC_CALL:
            // probably need a gen_params function for this to generate PARAM quads
            create_params_3ac(cur_node->child0); // process parameters (expr_list)
            Operand* func_ref = make_operand(OPERAND_ST_PTR, cur_node->st_ref, 0);
            this_quad = newinstr(GC_CALL, func_ref, NULL, NULL);
            this_quad->nargs = get_num_args(cur_node->st_ref);
            append_quad(this_quad);

            
            Symbol* v0_val = create_tmp();
            cur_node->st_ref = v0_val;
            Operand* v0 = make_operand(OPERAND_ST_PTR, v0_val, 0);
            this_quad = newinstr(GC_RETRIEVE, NULL, NULL, v0);
            append_quad(this_quad);
            break;

        case RETURN:
            if (cur_node->child0 == NULL){
                this_quad = newinstr(GC_RETURN_VOID, NULL, NULL, NULL);
                append_quad(this_quad);
            }
            else{
                Symbol* ret_val;
                create_3ac(cur_node->child0);
                Operand* ret_val_operand = make_operand(OPERAND_ST_PTR, cur_node->child0->st_ref, 0);
                this_quad = newinstr(GC_RETURN, ret_val_operand, NULL, NULL);
                append_quad(this_quad);
            }
            break;

        case EXPR_LIST:
            create_3ac(cur_node->child0);
            create_3ac(cur_node->child1);
            break;

        case IF:
            create_if_3ac(cur_node);
            break;

        case WHILE:
            while_depth++;
            char* while_label = (char*)malloc(sizeof(char)*15);
            sprintf(while_label, "WHILE%d", while_count);
            Quad* while_top_label = newinstr(GC_LABEL, NULL, NULL, NULL); while_top_label->label = strdup(while_label);
            append_quad(while_top_label);
            make_while_expr(cur_node);
            while_count++;
            create_3ac(cur_node->child1);
            while_count--;
            Quad* while_jump_quad = newinstr(GC_GOTO, NULL, NULL, NULL); while_jump_quad->label = strdup(while_label);
            append_quad(while_jump_quad);
            
            char* while_done_label = (char*)malloc(sizeof(char)*15);
            sprintf(while_done_label, "WHILEDONE%d", while_count);
            Quad* while_done_label_quad = newinstr(GC_LABEL, NULL, NULL, NULL); while_done_label_quad->label = strdup(while_done_label);
            append_quad(while_done_label_quad);

            while_count += while_depth;
            while_depth = 1;
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

void create_if_3ac(ASTnode* cur_node){
    // if (condition) goto TRUE
    nested_depth++;
    make_if_expr(cur_node);
    // else goto FALSE
    char* else_label = (char*)malloc(sizeof(char)*15);
    sprintf(else_label, "ELSE%d", label_count);
    Quad* jump_else = newinstr(GC_GOTO, NULL, NULL, NULL); jump_else->label = strdup(else_label);
    append_quad(jump_else);

    // TRUE BODY
    char* true_label = (char*)malloc(sizeof(char)*15);
    sprintf(true_label, "TRUE%d", label_count);
    Quad* true_label_quad = newinstr(GC_LABEL, NULL, NULL, NULL); true_label_quad->label = strdup(true_label);
    append_quad(true_label_quad); 
    label_count++;
    create_3ac(cur_node->child1);
    label_count--;

    char* after_label = (char*)malloc(sizeof(char)*15);
    sprintf(after_label, "AFTER%d", label_count);
    Quad* jump_after_quad = newinstr(GC_GOTO, NULL, NULL, NULL); jump_after_quad->label = strdup(after_label);
    append_quad(jump_after_quad);

    // ELSE BODY
    Quad* else_label_quad = newinstr(GC_LABEL, NULL, NULL, NULL); else_label_quad->label = strdup(else_label);
    append_quad(else_label_quad);
    label_count++;
    create_3ac(cur_node->child2);
    label_count--;
    Quad* jump_after_else_quad = newinstr(GC_GOTO, NULL, NULL, NULL); jump_after_else_quad->label = strdup(after_label);
    append_quad(jump_after_else_quad);

    // AFTER LABEL
    Quad* after_label_quad = newinstr(GC_LABEL, NULL, NULL, NULL); after_label_quad->label = strdup(after_label);
    append_quad(after_label_quad);
    label_count += nested_depth; // NEEDS TO BE INCREMENTED AS MANY TIMES AS THERE WERE NESTED IFS
    nested_depth = 1;
}



void make_if_expr(ASTnode* cur_node){
    cur_node = cur_node->child0;
    ASTnode* lh_node = cur_node->child0;
    ASTnode* rh_node = cur_node->child1;
    create_3ac(lh_node); create_3ac(rh_node); // rhs and lhs will be assigned temps if they're intconsts

    char* label = (char*)malloc(sizeof(char)*15);

    sprintf(label, "TRUE%d", label_count);

    switch(cur_node->node_type){
        Quad* relop_quad;

        case GT:
            Operand* lhs = make_operand(OPERAND_ST_PTR, lh_node->st_ref, 0);
            Operand* rhs = make_operand(OPERAND_ST_PTR, rh_node->st_ref, 0);
            relop_quad = newinstr(GC_GT, lhs, rhs, NULL);
            relop_quad->label = strdup(label);
            append_quad(relop_quad);
            break;

        case GE:
            lhs = make_operand(OPERAND_ST_PTR, lh_node->st_ref, 0);
            rhs = make_operand(OPERAND_ST_PTR, rh_node->st_ref, 0);
            relop_quad = newinstr(GC_GE, lhs, rhs, NULL);
            relop_quad->label = strdup(label);
            append_quad(relop_quad);
            break;

        case LT:
            lhs = make_operand(OPERAND_ST_PTR, lh_node->st_ref, 0);
            rhs = make_operand(OPERAND_ST_PTR, rh_node->st_ref, 0);
            relop_quad = newinstr(GC_LT, lhs, rhs, NULL);
            relop_quad->label = strdup(label);
            append_quad(relop_quad);
            break;

        case LE:
            lhs = make_operand(OPERAND_ST_PTR, lh_node->st_ref, 0);
            rhs = make_operand(OPERAND_ST_PTR, rh_node->st_ref, 0);
            relop_quad = newinstr(GC_LE, lhs, rhs, NULL);
            relop_quad->label = strdup(label);
            append_quad(relop_quad);
            break;
        
        case EQ:
            lhs = make_operand(OPERAND_ST_PTR, lh_node->st_ref, 0);
            rhs = make_operand(OPERAND_ST_PTR, rh_node->st_ref, 0);
            relop_quad = newinstr(GC_EQ, lhs, rhs, NULL);
            relop_quad->label = strdup(label);
            append_quad(relop_quad);
            break;

        case NE:
            lhs = make_operand(OPERAND_ST_PTR, lh_node->st_ref, 0);
            rhs = make_operand(OPERAND_ST_PTR, rh_node->st_ref, 0);
            relop_quad = newinstr(GC_NE, lhs, rhs, NULL);
            relop_quad->label = strdup(label);
            append_quad(relop_quad);
            break;
    }
}

void make_while_expr(ASTnode* cur_node){
    cur_node = cur_node->child0;
    ASTnode* lh_node = cur_node->child0;
    ASTnode* rh_node = cur_node->child1;
    create_3ac(lh_node); create_3ac(rh_node); // rhs and lhs will be assigned temps if they're intconsts

    char* label = (char*)malloc(sizeof(char)*15);

    sprintf(label, "WHILEDONE%d", while_count);

    switch(cur_node->node_type){
        Quad* relop_quad;
        case GT:
            Operand* lhs = make_operand(OPERAND_ST_PTR, lh_node->st_ref, 0);
            Operand* rhs = make_operand(OPERAND_ST_PTR, rh_node->st_ref, 0);
            relop_quad = newinstr(GC_LE, lhs, rhs, NULL);
            relop_quad->label = strdup(label);
            append_quad(relop_quad);
            break;

        case LT:
            lhs = make_operand(OPERAND_ST_PTR, lh_node->st_ref, 0);
            rhs = make_operand(OPERAND_ST_PTR, rh_node->st_ref, 0);
            relop_quad = newinstr(GC_GE, lhs, rhs, NULL);
            relop_quad->label = strdup(label);
            append_quad(relop_quad);
            break;
        
        case EQ:
            lhs = make_operand(OPERAND_ST_PTR, lh_node->st_ref, 0);
            rhs = make_operand(OPERAND_ST_PTR, rh_node->st_ref, 0);
            relop_quad = newinstr(GC_NE, lhs, rhs, NULL);
            relop_quad->label = strdup(label);
            append_quad(relop_quad);
            break;

        case LE:
            lhs = make_operand(OPERAND_ST_PTR, lh_node->st_ref, 0);
            rhs = make_operand(OPERAND_ST_PTR, rh_node->st_ref, 0);
            relop_quad = newinstr(GC_GT, lhs, rhs, NULL);
            relop_quad->label = strdup(label);
            append_quad(relop_quad);
            break;

        case GE:
            lhs = make_operand(OPERAND_ST_PTR, lh_node->st_ref, 0);
            rhs = make_operand(OPERAND_ST_PTR, rh_node->st_ref, 0);
            relop_quad = newinstr(GC_LT, lhs, rhs, NULL);
            relop_quad->label = strdup(label);
            append_quad(relop_quad);
            break;

        default:
            fprintf(stderr, "ERROR: Invalid while expression\n");
            exit(1);
            break;
    }


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

        case ADD:
            create_3ac(cur_node);
            Operand* add_param = make_operand(OPERAND_ST_PTR, cur_node->st_ref, 0);
            Quad* add_param_instr = newinstr(GC_PARAM, add_param, NULL, NULL);
            append_quad(add_param_instr);
            break;

        case SUB:
            create_3ac(cur_node);
            Operand* sub_param = make_operand(OPERAND_ST_PTR, cur_node->st_ref, 0);
            Quad* sub_param_instr = newinstr(GC_PARAM, sub_param, NULL, NULL);
            append_quad(sub_param_instr);
            break;

        case MUL:
            create_3ac(cur_node);
            Operand* mul_param = make_operand(OPERAND_ST_PTR, cur_node->st_ref, 0);
            Quad* mul_param_instr = newinstr(GC_PARAM, mul_param, NULL, NULL);
            append_quad(mul_param_instr);
            break;

        case DIV:
            create_3ac(cur_node);
            Operand* div_param = make_operand(OPERAND_ST_PTR, cur_node->st_ref, 0);
            Quad* div_param_instr = newinstr(GC_PARAM, div_param, NULL, NULL);
            append_quad(div_param_instr);
            break;

        case UMINUS:
            create_3ac(cur_node);
            Operand* uminus_param = make_operand(OPERAND_ST_PTR, cur_node->st_ref, 0);
            Quad* uminus_param_instr = newinstr(GC_PARAM, uminus_param, NULL, NULL);
            append_quad(uminus_param_instr);
            break;

        case FUNC_CALL:
            // if a function call is a parameter, we need to do the function call
            create_3ac(cur_node);

            // create a temp to store the return value
            Symbol* v0_val = create_tmp();
            Operand* v0 = make_operand(OPERAND_ST_PTR, v0_val, 0);
            Quad* this_quad = newinstr(GC_RETRIEVE, NULL, NULL, v0);
            append_quad(this_quad);

            // set return value as a param
            this_quad = newinstr(GC_PARAM, v0, NULL, NULL);
            append_quad(this_quad);

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